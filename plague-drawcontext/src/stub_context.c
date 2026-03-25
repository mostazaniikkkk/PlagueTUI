#include "stub_context.h"
#include <string.h>

static PG_DrawCommand g_commands[PG_STUB_MAX_COMMANDS];
static int            g_count = 0;

void pg_stub_reset(void) {
    g_count = 0;
}

int pg_stub_count(void) {
    return g_count;
}

PG_DrawCommand pg_stub_get(int index) {
    if (index < 0 || index >= g_count) {
        PG_DrawCommand empty = {0};
        return empty;
    }
    return g_commands[index];
}

static void push(PG_DrawCommand cmd) {
    if (g_count < PG_STUB_MAX_COMMANDS)
        g_commands[g_count++] = cmd;
}

void pg_stub_fill_rect(TG_Region region, PG_Color color) {
    PG_DrawCommand cmd = { .type = PG_CMD_FILL_RECT };
    cmd.fill_rect.region = region;
    cmd.fill_rect.color  = color;
    push(cmd);
}

void pg_stub_stroke_rect(TG_Region region, PG_Color color, float stroke_width) {
    PG_DrawCommand cmd = { .type = PG_CMD_STROKE_RECT };
    cmd.stroke_rect.region       = region;
    cmd.stroke_rect.color        = color;
    cmd.stroke_rect.stroke_width = stroke_width;
    push(cmd);
}

void pg_stub_draw_text(TG_Offset pos, const char* text, PG_TextStyle style) {
    PG_DrawCommand cmd = { .type = PG_CMD_DRAW_TEXT };
    cmd.draw_text.pos   = pos;
    cmd.draw_text.style = style;
    strncpy(cmd.draw_text.text, text, sizeof(cmd.draw_text.text) - 1);
    cmd.draw_text.text[sizeof(cmd.draw_text.text) - 1] = '\0';
    push(cmd);
}

void pg_stub_clip_push(TG_Region region) {
    PG_DrawCommand cmd = { .type = PG_CMD_CLIP_PUSH };
    cmd.clip_push.region = region;
    push(cmd);
}

void pg_stub_clip_pop(void) {
    PG_DrawCommand cmd = { .type = PG_CMD_CLIP_POP };
    push(cmd);
}

void pg_stub_translate_push(TG_Offset offset) {
    PG_DrawCommand cmd = { .type = PG_CMD_TRANSLATE_PUSH };
    cmd.translate_push.offset = offset;
    push(cmd);
}

void pg_stub_translate_pop(void) {
    PG_DrawCommand cmd = { .type = PG_CMD_TRANSLATE_POP };
    push(cmd);
}
