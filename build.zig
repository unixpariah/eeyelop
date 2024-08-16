const std = @import("std");

fn getWaylandProtocolsDir(alloc: std.mem.Allocator) ![]const u8 {
    var process = std.process.Child.init(&[_][]const u8{ "pkg-config", "wayland-protocols", "--variable=pkgdatadir" }, alloc);
    process.stdout_behavior = .Pipe;
    process.stderr_behavior = .Pipe;

    var stdout = std.ArrayList(u8).init(alloc);
    var stderr = std.ArrayList(u8).init(alloc);
    defer stderr.deinit();
    try process.spawn();
    try process.collectOutput(&stdout, &stderr, 4096);
    _ = try process.wait();

    return try stdout.toOwnedSlice();
}

fn genWaylandProtocol(
    alloc: std.mem.Allocator,
    wayland_scanner: []const u8,
    spec_location: []const u8,
    output_dir: []const u8,
) !void {
    var process = std.process.Child.init(&[_][]const u8{
        wayland_scanner,
        "client-header",
        spec_location,
        output_dir,
    }, alloc);
    _ = try process.spawnAndWait();
}

pub fn build(b: *std.Build) !void {
    const alloc = std.heap.page_allocator;

    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "eeylop",
        .target = target,
        .optimize = optimize,
    });

    exe.linkLibC();

    exe.addIncludePath(b.path("include"));
    exe.addCSourceFiles(.{ .files = &[_][]const u8{"src/main.c"}, .flags = &[_][]const u8{
        "-std=c11",
        "-pedantic",
        "-Wall",
        "-W",
        "-Wno-missing-field-initializers",
        "-fno-sanitize=undefined",
    } });

    const wayland_protocols_dir = try getWaylandProtocolsDir(alloc);
    defer alloc.free(wayland_protocols_dir);
    const xdg_shell_protocol = try std.fmt.allocPrint(
        alloc,
        "{s}/stable/xdg-shell/xdg-shell.xml",
        .{wayland_protocols_dir[0 .. wayland_protocols_dir.len - 1]},
    );
    defer alloc.free(xdg_shell_protocol);
    const xdg_output_protocol = try std.fmt.allocPrint(
        alloc,
        "{s}/unstable/xdg-output/xdg-output-unstable-v1.xml",
        .{wayland_protocols_dir[0 .. wayland_protocols_dir.len - 1]},
    );
    defer alloc.free(xdg_output_protocol);

    const wayland_scanner = try b.findProgram(&[_][]const u8{"wayland-scanner"}, &[_][]const u8{"wayland-scanner"});
    try genWaylandProtocol(
        alloc,
        wayland_scanner,
        "protocols/wlr-layer-shell-unstable-v1.xml",
        "include/wlr-layer-shell-protocol.h",
    );
    try genWaylandProtocol(
        alloc,
        wayland_scanner,
        xdg_shell_protocol,
        "include/xdg-shell-protocol.h",
    );
    try genWaylandProtocol(
        alloc,
        wayland_scanner,
        xdg_output_protocol,
        "include/xdg-output-protocol.h",
    );

    exe.linkSystemLibrary("wayland-client");
    exe.linkSystemLibrary("wayland-protocols");
    exe.linkSystemLibrary("xkbcommon");
    exe.linkSystemLibrary("wayland-egl");
    exe.linkSystemLibrary("lua");
    exe.linkSystemLibrary("egl");
    exe.linkSystemLibrary("gl");
    exe.linkSystemLibrary("freetype2");
    exe.linkSystemLibrary("fontconfig");

    b.installArtifact(exe);

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());

    const run_step = b.step("run", "Run");
    run_step.dependOn(&run_cmd.step);
}
