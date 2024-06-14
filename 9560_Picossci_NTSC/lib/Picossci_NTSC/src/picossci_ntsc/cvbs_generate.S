.align  4
.code   16
.syntax unified


  .thumb_func
  .global cvbs_generate_blit_8bit

cvbs_generate_blit_8bit:
        push    {r4, r5, r6, r7, lr}
        mov     lr, r11
        mov     r7, r10
        mov     r6, r9
        mov     r5, r8
        push    {r5, r6, r7, lr}

        mov     r8, r1          // r8 = d_end
        ldr     r7, [sp, #36]
        mov     r9, r7          // r9 = scale
        ldr     r6, [sp, #40]   // r6 = diff
        lsls    r7, r7, #2
        mov     r10, r7         // r10 = scale*4

        ldrb    r1, [r2]        // r7 = s[0]

        b       LABEL_BLIT8_ENTER

LABEL_BLIT8_ALIGN1:
        lsrs    r7, r5, #8
        strb    r7, [r0]
        adds    r0, r0, #1
LABEL_BLIT8_ALIGN2:
        lsrs    r7, r5, #16
        strh    r7, [r0]
        adds    r0, r0, #2
LABEL_BLIT8_ALIGN0:
        asrs    r7, r6, #8
        add     r7, r8, r7    // r7 に書き込み範囲の終端を設定

        cmp     r7, r8        // 終端に達したかチェック
        bge     LABEL_BLIT8_EXIT

LABEL_BLIT8_DST_LOOP:
        str     r5, [r0]
        str     r5, [r0, #4]
        str     r5, [r0, #8]
        adds    r0, r0, #12
        cmp     r0, r7
        bcc     LABEL_BLIT8_DST_LOOP
        movs    r0, r7

LABEL_BLIT8_ENTER:

LABEL_BLIT8_SRC_LOOP_START:  // 色が変わる箇所を探す。効率を上げるためループ一回で4ピクセル分を処理
        ldrb    r5, [r2, #1]
        cmp     r1, r5
        bne     LABEL_BLIT8_SRC_LOOP_END1
        ldrb    r5, [r2, #2]
        cmp     r1, r5
        bne     LABEL_BLIT8_SRC_LOOP_END2
        ldrb    r5, [r2, #3]
        cmp     r1, r5
        bne     LABEL_BLIT8_SRC_LOOP_END3
        ldrb    r5, [r2, #4]
        adds    r2, r2, #4
        add     r6, r6, r10
        cmp     r1, r5
        bne     LABEL_BLIT8_SRC_LOOP_END
        cmp     r6, #0
        blt     LABEL_BLIT8_SRC_LOOP_START
        b       LABEL_BLIT8_SRC_LOOP_END
LABEL_BLIT8_SRC_LOOP_END3:
        adds    r2, r2, #1
        add     r6, r6, r9
LABEL_BLIT8_SRC_LOOP_END2:
        adds    r2, r2, #1
        add     r6, r6, r9
LABEL_BLIT8_SRC_LOOP_END1:
        adds    r2, r2, #1
        add     r6, r6, r9
LABEL_BLIT8_SRC_LOOP_END:
        lsls    r7, r1, #2
        mov     r1, r5              // 次の周回で使用するためにピクセル色をr1に格納
        ldr     r5, [r3, r7]    // r5に下位8bitから得た波形を格納

        lsls    r7, r0, #30
        lsrs    r7, r7, #30
        cmp     r7, #1          // 下位2bitの状況に応じて分岐
        blt     LABEL_BLIT8_ALIGN0
        beq     LABEL_BLIT8_ALIGN1
        cmp     r7, #2
        beq     LABEL_BLIT8_ALIGN2

LABEL_BLIT8_ALIGN3:
        lsrs    r7, r5, #24
        strb    r7, [r0]
        adds    r0, r0, #1
        b       LABEL_BLIT8_ALIGN0

LABEL_BLIT8_EXIT:
        str     r5, [r0]       // 最後のピクセルを書き込む
        str     r5, [r0, #4]
        adds    r0, r0, #8
        cmp     r0, r8
        bcc     LABEL_BLIT8_EXIT // 終端に達するまで繰り返す

        pop     {r4, r5, r6, r7}
        mov     r11, r7
        mov     r10, r6
        mov     r9, r5
        mov     r8, r4
        pop     {r4, r5, r6, r7, pc}
//*/


  .thumb_func
  .global cvbs_generate_blit_16bit

cvbs_generate_blit_16bit:
        push    {r4, r5, r6, r7, lr}
        mov     r6, r9
        mov     r5, r8
        push    {r5, r6}

        ldr     r7, [sp, #28]
        mov     r9, r7          // r9 = scale
        ldr     r6, [sp, #32]   // r6 = diff
        lsls    r7, r7, #2
        mov     r8, r7          // r8 = scale*4

        movs    r7, #1
        lsls    r7, r7, #10
        adds    r4, r3, r7      // r4に上位8bit用のパレットテーブルのアドレスを格納

        b       LABEL_BLIT16_ENTER

LABEL_BLIT16_ALIGN1:
        lsrs    r7, r5, #8
        strb    r7, [r0]
        adds    r0, r0, #1
LABEL_BLIT16_ALIGN2:
        lsrs    r7, r5, #16
        strh    r7, [r0]
        adds    r0, r0, #2
LABEL_BLIT16_ALIGN0:
        asrs    r7, r6, #8
        adds    r7, r1, r7    // r7 に書き込み範囲の終端を設定

        cmp     r7, r1        // 終端に達したかチェック
        bge     LABEL_BLIT16_EXIT

LABEL_BLIT16_DST_LOOP:
        str     r5, [r0]
        str     r5, [r0, #4]
        str     r5, [r0, #8]
        adds    r0, r0, #12
        cmp     r0, r7
        bcc     LABEL_BLIT16_DST_LOOP
        movs    r0, r7

LABEL_BLIT16_ENTER:
        ldrh    r7, [r2]        // r7 = s[0]

LABEL_BLIT16_SRC_LOOP_START:  // 色が変わる箇所を探す。効率を上げるためループ一回で4ピクセル分を処理
        ldrh    r5, [r2, #2]
        cmp     r7, r5
        bne     LABEL_BLIT16_SRC_LOOP_END1
        ldrh    r5, [r2, #4]
        cmp     r7, r5
        bne     LABEL_BLIT16_SRC_LOOP_END2
        ldrh    r5, [r2, #6]
        cmp     r7, r5
        bne     LABEL_BLIT16_SRC_LOOP_END3
        ldrh    r5, [r2, #8]
        adds    r2, r2, #8
        add     r6, r6, r8
        cmp     r7, r5
        bne     LABEL_BLIT16_SRC_LOOP_END
        cmp     r6, #0
        blt     LABEL_BLIT16_SRC_LOOP_START
        b       LABEL_BLIT16_SRC_LOOP_END
LABEL_BLIT16_SRC_LOOP_END3:
        adds    r2, r2, #2
        add     r6, r6, r9
LABEL_BLIT16_SRC_LOOP_END2:
        adds    r2, r2, #2
        add     r6, r6, r9
LABEL_BLIT16_SRC_LOOP_END1:
        adds    r2, r2, #2
        add     r6, r6, r9
LABEL_BLIT16_SRC_LOOP_END:
        lsls    r5, r7, #24     // 下位8bitを取り出す
        lsrs    r5, r5, #22
        ldr     r5, [r3, r5]    // r5に下位8bitから得た波形を格納
        lsrs    r7, r7, #8      // 上位8bitを取り出す
        lsls    r7, r7, #2
        ldr     r7, [r4, r7]    // r7に上位8bitから得た波形を格納
        adds    r5, r7, r5      // 上位8bitと下位8bitを合成

        lsls    r7, r0, #30
        lsrs    r7, r7, #30
        cmp     r7, #1          // 下位2bitの状況に応じて分岐
        blt     LABEL_BLIT16_ALIGN0
        beq     LABEL_BLIT16_ALIGN1
        cmp     r7, #2
        beq     LABEL_BLIT16_ALIGN2

LABEL_BLIT16_ALIGN3:
        lsrs    r7, r5, #24
        strb    r7, [r0]
        adds    r0, r0, #1
        b       LABEL_BLIT16_ALIGN0

LABEL_BLIT16_EXIT:
        str     r5, [r0]       // 最後のピクセルを書き込む
        str     r5, [r0, #4]
        adds    r0, r0, #8
        cmp     r0, r1
        bcc     LABEL_BLIT16_EXIT // 終端に達するまで繰り返す

        pop     {r4, r5}
        mov     r9, r5
        mov     r8, r4
        pop     {r4, r5, r6, r7, pc}
