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

fn generateProtocolCode(
    b: *std.Build,
    run_cmd: *std.Build.Step.Run,
    protocol_path: []const u8,
    comptime protocol_name: []const u8,
) void {
    const client_header = b.addSystemCommand(&.{
        "wayland-scanner",
        "client-header",
        protocol_path,
        "include/" ++ protocol_name ++ ".h",
    });

    const private_code = b.addSystemCommand(&.{
        "wayland-scanner",
        "private-code",
        protocol_path,
        "lib/" ++ protocol_name ++ ".c",
    });

    run_cmd.step.dependOn(&client_header.step);
    run_cmd.step.dependOn(&private_code.step);
}

pub fn build(b: *std.Build) !void {
    const alloc = std.heap.page_allocator;

    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "eeyelop",
        .target = target,
        .optimize = optimize,
    });

    exe.linkLibC();

    exe.addIncludePath(b.path("include"));
    exe.addLibraryPath(b.path("lib"));

    const source_files = &[_][]const u8{
        "lib/ArrayList.c",
        "src/Output.c",
        "src/main.c",
        "src/Eeyelop.c",
        "src/Config.c",
        "src/Seat.c",
        "src/math.c",
        "src/Notification.c",
        "src/Text.c",
    };

    const protocol_files = &[_][]const u8{
        "lib/xdg-shell-client-protocol.c",
        "lib/xdg-output-client-protocol.c",
        "lib/wlr-layer-shell-unstable-v1-client-protocol.c",
    };

    const flags = &[_][]const u8{
        "-std=c11",
        "-pedantic",
        "-Wall",
        "-W",
        "-Wno-missing-field-initializers",
        "-fno-sanitize=undefined",
        "-Wunused-result",
    };

    exe.addCSourceFiles(.{
        .files = source_files ++ protocol_files,
        .flags = flags,
    });

    const wayland_protocols_dir = try getWaylandProtocolsDir(alloc);
    defer alloc.free(wayland_protocols_dir);

    const wlr_layer_shell_protocol = "protocols/wlr-layer-shell-unstable-v1.xml";
    const xdg_shell_protocol = try std.fmt.allocPrint(
        alloc,
        "{s}/stable/xdg-shell/xdg-shell.xml",
        .{wayland_protocols_dir[0 .. wayland_protocols_dir.len - 1]},
    );
    defer alloc.free(xdg_shell_protocol);

    exe.linkSystemLibrary("wayland-client");
    exe.linkSystemLibrary("wayland-egl");
    exe.linkSystemLibrary("egl");
    exe.linkSystemLibrary("gl");
    exe.linkSystemLibrary("fontconfig");
    exe.linkSystemLibrary("freetype2");

    b.installArtifact(exe);

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());

    generateProtocolCode(b, run_cmd, xdg_shell_protocol, "xdg-shell-client-protocol");
    generateProtocolCode(b, run_cmd, wlr_layer_shell_protocol, "wlr-layer-shell-unstable-v1-client-protocol");

    if (b.args) |args| run_cmd.addArgs(args);

    const run_step = b.step("run", "Run");
    run_step.dependOn(&run_cmd.step);

    const clang_tidy_cmd = b.addSystemCommand(.{"clang-tidy"} ++ source_files);
    const tidy_step = b.step("tidy", "Run clang-tidy");
    tidy_step.dependOn(&clang_tidy_cmd.step);

    const valgrind_cmd = b.addSystemCommand(&.{ "valgrind", "--leak-check=full", "./zig-out/bin/eeyelop" });

    const valgrind_step = b.step("valgrind", "Run valgrind");
    valgrind_step.dependOn(&valgrind_cmd.step);
}
