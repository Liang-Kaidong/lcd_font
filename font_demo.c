#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include "lcd_font.h"


int main() {
    /* 初始化字库 */ 
    if (lcd_init("/dev/fb0", "simkai.ttf") != 0) {
        printf("初始化失败.\n");
        return -1;
    }

    /* 清屏 */
    lcd_clear(COLOR_BLACK);

    /* 测试普通文本渲染（使用数组的方式寄存文字） */ 
    const char text[] = "普通文本渲染";
    lcd_render_text(
        text,              /* 文本内容 */
        50, 50,            /* 起始坐标 (x, y) */
        COLOR_WHITE,       /* 文本颜色 */
        45                 /* 字体大小 */
    ); 

    /* 测试矩形文本框 */ 
    lcd_render_text_with_box(
        "矩形文本框",            /* 文本内容 */
        50, 100,                /* 起始坐标 (x, y) */
        COLOR_YELLOW,           /* 文本颜色 */ 
        COLOR_BLUE,             /* 文本框背景颜色 */ 
        0,                      /* 文本与文本框边缘的间距 */ 
        BOX_STYLE_RECTANGLE,    /* 矩形样式 */ 
        0,                      /* 矩形样式不需要半径 */
        50,                     /* 字体大小 */
        400,                    /* 文本框宽度 */
        50                      /* 文本框高度 */
    );

    /* 测试圆角文本框（不同半径） */ 
    lcd_render_text_with_box(
        "标准文字自动居中文本框 (r=10)",     /* 文本内容 */
        50, 170,                           /* 起始坐标 (x, y) */
        COLOR_WHITE,                       /* 文本颜色 */
        COLOR_RED,                         /* 文本框背景颜色 */
        10,                                /* 文本与文本框边缘的间距 */
        BOX_STYLE_ROUNDED,                 /* 圆角样式 */
        15,                                /* 圆角半径 */
        25,                                /* 字体大小 */
        0,                                 /* 文本框宽度，为 0 时，文本框大小依照文字大小与文本量大小调整，文字居中对齐 */
        0                                  /* 文本框高度，为 0 时，文本框大小依照文字大小与文本量大小调整，文字居中对齐 */
    );

    lcd_render_text_with_box(
        "中等圆角文本框，不随文字居中 (r=20)",            /* 文本内容 */
        50, 220,                                        /* 起始坐标 (x, y) */
        COLOR_WHITE,                                    /* 文本颜色 */
        COLOR_MAGENTA,                                  /* 文本框背景颜色 */
        10,                                             /* 文本与文本框边缘的间距 */
        BOX_STYLE_ROUNDED,                              /* 圆角样式 */
        20,                                             /* 圆角半径 */
        20,                                             /* 字体大小 */
        450,                                            /* 文本框宽度 */
        40                                              /* 文本框高度 */
    );

    lcd_render_text_with_box(
        "大圆角文本框，改变文字与文本框间距(r=30)",            /* 文本内容 */
        55, 280,                                            /* 起始坐标 (x, y) */
        COLOR_BLACK,                                        /* 文本颜色 */
        COLOR_CYAN,                                         /* 文本框背景颜色 */
        10,                                                 /* 文本与文本框边缘的间距 */
        BOX_STYLE_ROUNDED,                                  /* 圆角样式 */
        30,                                                 /* 圆角半径 */
        20,                                                 /* 字体大小 */
        450,                                                /* 文本框宽度 */
        60                                                  /* 文本框高度 */
    );

    /* 测试低级别图形API - 直接绘制矩形 */ 
    /* 绘制空心矩形 */
    lcd_draw_rectangle(
        50, 350,        /* 左上角坐标 (x, y) */
        200, 60,        /* 矩形宽度和高度 */
        COLOR_GREEN     /* 矩形边框颜色 */
    );

    /* 绘制填充矩形 */
    lcd_draw_filled_rectangle(
        300, 350,       /* 左上角坐标 (x, y) */
        200, 60,        /* 矩形宽度和高度 */
        COLOR_GREEN);   /* 填充颜色 */

    /* 测试低级别图形API - 直接绘制圆角矩形 */ 
    /* 绘制空心圆角矩形 */
    lcd_draw_rounded_rectangle(
        50, 420,        /* 左上角坐标 (x, y) */
        200, 60,        /* 矩形宽度和高度 */
        15,             /* 圆角半径 */
        COLOR_YELLOW    /* 圆角边框颜色 */
    );

    /* 绘制填充圆角矩形 */
    lcd_draw_filled_rounded_rectangle(
        300, 420,       /* 左上角坐标 (x, y) */
        200, 60,        /* 矩形宽度和高度 */
        25,             /* 圆角半径 */
        COLOR_YELLOW    /* 填充颜色 */
    );

    /* 这里可以自己修改字体位置和圆角矩形效果，不用受制于标准的字体位置 */
    /* 测试大圆角效果（接近半圆） */ 
    lcd_draw_filled_rounded_rectangle(
        50, 500,             /* 左上角坐标 (x, y) */
        850, 80,             /* 矩形宽度和高度 */
        40,                  /* 圆角半径 */
        COLOR_PURPLE         /* 填充颜色 */
    );
    lcd_render_text(
        "超大圆角矩形示例,文字位置可自定义",   /* 文本内容 */
        160, 500,                           /* 起始坐标 (x, y) */
        COLOR_WHITE,                        /* 文本颜色 */
        40                                  /* 字体大小 */
    );

    /* 测试极端情况：正方形 + 圆角半径 = 边长/2 = 半圆 */ 
    lcd_draw_filled_rounded_rectangle(
        520, 100,                           /* 左上角坐标 (x, y) */ 
        100, 100,                           /* 矩形宽度和高度 */
        50,                                 /* 圆角半径 */
        COLOR_WHITE                         /* 填充颜色 */
    );
    lcd_render_text(
        "类似全圆",                          /* 文本内容 */
        530, 140,                           /* 起始坐标 (x, y) */
        COLOR_BLACK,                        /* 文本颜色 */
        20                                  /* 字体大小 */
    );

    /* 测试中文显示 */ 
    lcd_render_text_with_box(
        "圆角矩形中文测试，无文本框间距", 520, 220,    /* 文本内容 */
        COLOR_WHITE,                                /* 文本颜色 */
        COLOR_DARKGREEN,                            /* 文本框背景颜色 */
        0,                                          /* 文本与文本框边缘的间距 */
        BOX_STYLE_ROUNDED,                          /* 圆角样式 */
        15,                                         /* 圆角半径 */
        24,                                         /* 字体大小 */
        0,                                          /* 文本框宽度 */
        0                                           /* 文本框高度 */
    );

    /* 测试不同颜色组合 */
    lcd_render_text_with_box(
        "多种颜色效果", 520, 280,        /* 文本内容 */
        COLOR_BLACK,                    /* 文本颜色 */
        COLOR_DARKBLUE,                 /* 文本框背景颜色 */
        5,                              /* 文本与文本框边缘的间距 */
        BOX_STYLE_ROUNDED,              /* 圆角样式 */
        12,                             /* 圆角半径 */
        30,                             /* 字体大小 */
        0,                              /* 文本框宽度 */
        0                               /* 文本框高度 */
    );

    /* 测试多行文本提示（需自行实现换行逻辑） */ 
    lcd_draw_filled_rounded_rectangle(
        520, 340,                           /* 左上角坐标 (x, y) */
        240, 140,                           /* 矩形宽度和高度 */
        15,                                 /* 圆角半径 */
        COLOR_LIGHTGRAY                     /* 填充颜色 */
    );
    lcd_render_text("第一行文本", 535, 360, COLOR_RED, 40);
    lcd_render_text("第二行文本", 535, 390, COLOR_RED, 40);
    lcd_render_text("第三行文本", 535, 420, COLOR_RED, 40);

    /* 署名测试 */ 
    lcd_render_text("电科1223", 800, 340, COLOR_WHITE, 40);
    lcd_render_text_with_box(
        "Liang Kaidong",                    /* 文本内容 */
        805, 400,                           /* 起始坐标 (x, y) */
        COLOR_BLACK,                        /* 文本颜色 */
        COLOR_LIGHTGRAY,                    /* 文本框背景颜色 */
        10,                                 /* 文本与文本框边缘的间距 */
        BOX_STYLE_ROUNDED,                  /* 圆角样式 */
        15,                                 /* 圆角半径 */
        25,                                 /* 字体大小 */
        0,                                  /* 文本框宽度 */
        0                                   /* 文本框高度 */
    );

    /* 等待1秒以便查看结果 */ 
    sleep(2);

    /* 清屏 */
    //lcd_clear(COLOR_BLACK);

    /* 清理资源 */ 
    lcd_cleanup();

    return 0;
}