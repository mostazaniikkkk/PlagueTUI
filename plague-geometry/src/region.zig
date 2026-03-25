// Region — rectángulo posicionado en celdas de terminal (i32)

const offset_mod = @import("offset.zig");
const size_mod   = @import("size.zig");

pub const Offset = offset_mod.Offset;
pub const Size   = size_mod.Size;

pub const Region = extern struct {
    x:      i32,
    y:      i32,
    width:  i32,
    height: i32,
};

pub const REGION_ZERO = Region{ .x = 0, .y = 0, .width = 0, .height = 0 };

pub fn region_contains(r: Region, o: Offset) bool {
    return o.x >= r.x and o.x <= r.x + r.width and
           o.y >= r.y and o.y <= r.y + r.height;
}

pub fn region_clip(a: Region, b: Region) Region {
    const x      = @max(a.x, b.x);
    const y      = @max(a.y, b.y);
    const right  = @min(a.x + a.width,  b.x + b.width);
    const bottom = @min(a.y + a.height, b.y + b.height);
    const w = right  - x;
    const h = bottom - y;
    if (w <= 0 or h <= 0) return REGION_ZERO;
    return .{ .x = x, .y = y, .width = w, .height = h };
}

pub fn region_union(a: Region, b: Region) Region {
    const x      = @min(a.x, b.x);
    const y      = @min(a.y, b.y);
    const right  = @max(a.x + a.width,  b.x + b.width);
    const bottom = @max(a.y + a.height, b.y + b.height);
    return .{ .x = x, .y = y, .width = right - x, .height = bottom - y };
}

pub fn region_translate(r: Region, o: Offset) Region {
    return .{ .x = r.x + o.x, .y = r.y + o.y, .width = r.width, .height = r.height };
}

pub fn region_is_empty(r: Region) bool {
    return r.width <= 0 or r.height <= 0;
}

pub fn region_center(r: Region) Offset {
    return .{ .x = r.x + @divTrunc(r.width, 2), .y = r.y + @divTrunc(r.height, 2) };
}

pub fn region_size(r: Region) Size {
    return .{ .width = r.width, .height = r.height };
}

pub fn region_eq(a: Region, b: Region) bool {
    return a.x == b.x and a.y == b.y and a.width == b.width and a.height == b.height;
}
