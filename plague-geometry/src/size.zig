// Size — dimensiones 2D en celdas de terminal (i32)

pub const Size = extern struct {
    width:  i32,
    height: i32,
};

pub const SIZE_ZERO = Size{ .width = 0, .height = 0 };

pub fn size_add(a: Size, b: Size) Size {
    return .{ .width = a.width + b.width, .height = a.height + b.height };
}

pub fn size_scale(a: Size, factor: i32) Size {
    return .{ .width = a.width * factor, .height = a.height * factor };
}

pub fn size_area(a: Size) i32 {
    return a.width * a.height;
}

pub fn size_eq(a: Size, b: Size) bool {
    return a.width == b.width and a.height == b.height;
}
