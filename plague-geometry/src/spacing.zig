// Spacing — márgenes o padding en celdas de terminal (i32)

pub const Spacing = extern struct {
    top:    i32,
    right:  i32,
    bottom: i32,
    left:   i32,
};

pub const SPACING_ZERO = Spacing{ .top = 0, .right = 0, .bottom = 0, .left = 0 };

pub fn spacing_uniform(value: i32) Spacing {
    return .{ .top = value, .right = value, .bottom = value, .left = value };
}

pub fn spacing_add(a: Spacing, b: Spacing) Spacing {
    return .{
        .top    = a.top    + b.top,
        .right  = a.right  + b.right,
        .bottom = a.bottom + b.bottom,
        .left   = a.left   + b.left,
    };
}

pub fn spacing_eq(a: Spacing, b: Spacing) bool {
    return a.top == b.top and a.right == b.right and
           a.bottom == b.bottom and a.left == b.left;
}
