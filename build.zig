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
    comptime output_name: []const u8,
) !void {
    {
        var process = std.process.Child.init(&[_][]const u8{
            wayland_scanner,
            "client-header",
            spec_location,
            "include/" ++ output_name ++ ".h",
        }, alloc);
        _ = try process.spawnAndWait();
    }

    {
        var process = std.process.Child.init(&[_][]const u8{
            wayland_scanner,
            "private-code",
            spec_location,
            "lib/" ++ output_name ++ ".c",
        }, alloc);
        _ = try process.spawnAndWait();
    }
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
    exe.addLibraryPath(b.path("lib"));
    exe.addCSourceFiles(.{ .files = &[_][]const u8{
        "lib/xdg-shell-client-protocol.c",
        "lib/xdg-output-client-protocol.c",
        "lib/wlr-layer-shell-unstable-v1-client-protocol.c",
        "lib/ArrayList.c",
        "src/Output.c",
        "src/main.c",
        "src/Eeyelop.c",
        "src/Egl.c",
    }, .flags = &[_][]const u8{
        "-std=c11",
        "-pedantic",
        "-Wall",
        "-W",
        "-Wno-missing-field-initializers",
        "-fno-sanitize=undefined",
        "-Wunused-result",
    } });

    const wayland_protocols_dir = try getWaylandProtocolsDir(alloc);
    defer alloc.free(wayland_protocols_dir);

    const wayland_scanner = try b.findProgram(&[_][]const u8{"wayland-scanner"}, &[_][]const u8{"wayland-scanner"});

    const wlr_layer_shell_protocol = "protocols/wlr-layer-shell-unstable-v1.xml";
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

    try genWaylandProtocol(alloc, wayland_scanner, wlr_layer_shell_protocol, "wlr-layer-shell-unstable-v1-client-protocol");
    try genWaylandProtocol(alloc, wayland_scanner, xdg_shell_protocol, "xdg-shell-client-protocol");
    try genWaylandProtocol(alloc, wayland_scanner, xdg_output_protocol, "xdg-output-client-protocol");

    exe.linkSystemLibrary("wayland-client");
    exe.linkSystemLibrary("wayland-egl");
    exe.linkSystemLibrary("egl");
    exe.linkSystemLibrary("gl");

    b.installArtifact(exe);

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| run_cmd.addArgs(args);

    const run_step = b.step("run", "Run");
    run_step.dependOn(&run_cmd.step);
}
