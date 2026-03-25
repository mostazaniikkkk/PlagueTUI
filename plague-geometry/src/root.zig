// plague-geometry — entry point y exports C ABI

const offset_mod  = @import("offset.zig");
const size_mod    = @import("size.zig");
const region_mod  = @import("region.zig");
const spacing_mod = @import("spacing.zig");

pub const Offset       = offset_mod.Offset;
pub const OFFSET_ZERO  = offset_mod.OFFSET_ZERO;
pub const Size         = size_mod.Size;
pub const SIZE_ZERO    = size_mod.SIZE_ZERO;
pub const Region       = region_mod.Region;
pub const REGION_ZERO  = region_mod.REGION_ZERO;
pub const Spacing      = spacing_mod.Spacing;
pub const SPACING_ZERO = spacing_mod.SPACING_ZERO;

// --- Offset ---

export fn tg_offset_add(a: offset_mod.Offset, b: offset_mod.Offset) offset_mod.Offset {
    return offset_mod.offset_add(a, b);
}

export fn tg_offset_sub(a: offset_mod.Offset, b: offset_mod.Offset) offset_mod.Offset {
    return offset_mod.offset_sub(a, b);
}

export fn tg_offset_scale(a: offset_mod.Offset, factor: i32) offset_mod.Offset {
    return offset_mod.offset_scale(a, factor);
}

export fn tg_offset_eq(a: offset_mod.Offset, b: offset_mod.Offset) bool {
    return offset_mod.offset_eq(a, b);
}

// --- Size ---

export fn tg_size_add(a: size_mod.Size, b: size_mod.Size) size_mod.Size {
    return size_mod.size_add(a, b);
}

export fn tg_size_scale(a: size_mod.Size, factor: i32) size_mod.Size {
    return size_mod.size_scale(a, factor);
}

export fn tg_size_area(a: size_mod.Size) i32 {
    return size_mod.size_area(a);
}

export fn tg_size_eq(a: size_mod.Size, b: size_mod.Size) bool {
    return size_mod.size_eq(a, b);
}

// --- Region ---

export fn tg_region_contains(r: region_mod.Region, o: region_mod.Offset) bool {
    return region_mod.region_contains(r, o);
}

export fn tg_region_clip(a: region_mod.Region, b: region_mod.Region) region_mod.Region {
    return region_mod.region_clip(a, b);
}

export fn tg_region_union(a: region_mod.Region, b: region_mod.Region) region_mod.Region {
    return region_mod.region_union(a, b);
}

export fn tg_region_translate(r: region_mod.Region, o: region_mod.Offset) region_mod.Region {
    return region_mod.region_translate(r, o);
}

export fn tg_region_inflate(r: region_mod.Region, s: spacing_mod.Spacing) region_mod.Region {
    return .{
        .x      = r.x      - s.left,
        .y      = r.y      - s.top,
        .width  = r.width  + s.left + s.right,
        .height = r.height + s.top  + s.bottom,
    };
}

export fn tg_region_deflate(r: region_mod.Region, s: spacing_mod.Spacing) region_mod.Region {
    const w = r.width  - s.left - s.right;
    const h = r.height - s.top  - s.bottom;
    return .{
        .x      = r.x + s.left,
        .y      = r.y + s.top,
        .width  = if (w > 0) w else 0,
        .height = if (h > 0) h else 0,
    };
}

export fn tg_region_is_empty(r: region_mod.Region) bool {
    return region_mod.region_is_empty(r);
}

export fn tg_region_center(r: region_mod.Region) region_mod.Offset {
    return region_mod.region_center(r);
}

export fn tg_region_size(r: region_mod.Region) region_mod.Size {
    return region_mod.region_size(r);
}

export fn tg_region_eq(a: region_mod.Region, b: region_mod.Region) bool {
    return region_mod.region_eq(a, b);
}

// --- Spacing ---

export fn tg_spacing_uniform(value: i32) spacing_mod.Spacing {
    return spacing_mod.spacing_uniform(value);
}

export fn tg_spacing_add(a: spacing_mod.Spacing, b: spacing_mod.Spacing) spacing_mod.Spacing {
    return spacing_mod.spacing_add(a, b);
}

export fn tg_spacing_eq(a: spacing_mod.Spacing, b: spacing_mod.Spacing) bool {
    return spacing_mod.spacing_eq(a, b);
}
