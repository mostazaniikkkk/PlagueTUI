// Offset — posición o desplazamiento 2D en celdas de terminal (i32)

pub const Offset = extern struct {
    x: i32,
    y: i32,
};

pub const OFFSET_ZERO = Offset{ .x = 0, .y = 0 };

pub fn offset_add(a: Offset, b: Offset) Offset {
    return .{ .x = a.x + b.x, .y = a.y + b.y };
}

pub fn offset_sub(a: Offset, b: Offset) Offset {
    return .{ .x = a.x - b.x, .y = a.y - b.y };
}

pub fn offset_scale(a: Offset, factor: i32) Offset {
    return .{ .x = a.x * factor, .y = a.y * factor };
}

pub fn offset_eq(a: Offset, b: Offset) bool {
    return a.x == b.x and a.y == b.y;
}
