nanodistro_inc += include_directories('include')
lvgl_srcs = files([
	'lvgl/src/core/lv_group.c',
	'lvgl/src/core/lv_obj.c',
	'lvgl/src/core/lv_obj_class.c',
	'lvgl/src/core/lv_obj_draw.c',
	'lvgl/src/core/lv_obj_event.c',
	'lvgl/src/core/lv_obj_id_builtin.c',
	'lvgl/src/core/lv_obj_pos.c',
	'lvgl/src/core/lv_obj_property.c',
	'lvgl/src/core/lv_obj_scroll.c',
	'lvgl/src/core/lv_obj_style.c',
	'lvgl/src/core/lv_obj_style_gen.c',
	'lvgl/src/core/lv_obj_tree.c',
	'lvgl/src/core/lv_refr.c',
	'lvgl/src/display/lv_display.c',
	'lvgl/src/draw/dma2d/lv_draw_dma2d.c',
	'lvgl/src/draw/dma2d/lv_draw_dma2d_fill.c',
	'lvgl/src/draw/dma2d/lv_draw_dma2d_img.c',
	'lvgl/src/draw/lv_draw_3d.c',
	'lvgl/src/draw/lv_draw_arc.c',
	'lvgl/src/draw/lv_draw_buf.c',
	'lvgl/src/draw/lv_draw.c',
	'lvgl/src/draw/lv_draw_image.c',
	'lvgl/src/draw/lv_draw_label.c',
	'lvgl/src/draw/lv_draw_line.c',
	'lvgl/src/draw/lv_draw_mask.c',
	'lvgl/src/draw/lv_draw_rect.c',
	'lvgl/src/draw/lv_draw_triangle.c',
	'lvgl/src/draw/lv_draw_vector.c',
	'lvgl/src/draw/lv_image_decoder.c',
	'lvgl/src/draw/nema_gfx/lv_draw_nema_gfx_arc.c',
	'lvgl/src/draw/nema_gfx/lv_draw_nema_gfx_border.c',
	'lvgl/src/draw/nema_gfx/lv_draw_nema_gfx.c',
	'lvgl/src/draw/nema_gfx/lv_draw_nema_gfx_fill.c',
	'lvgl/src/draw/nema_gfx/lv_draw_nema_gfx_img.c',
	'lvgl/src/draw/nema_gfx/lv_draw_nema_gfx_label.c',
	'lvgl/src/draw/nema_gfx/lv_draw_nema_gfx_layer.c',
	'lvgl/src/draw/nema_gfx/lv_draw_nema_gfx_line.c',
	'lvgl/src/draw/nema_gfx/lv_draw_nema_gfx_stm32_hal.c',
	'lvgl/src/draw/nema_gfx/lv_draw_nema_gfx_triangle.c',
	'lvgl/src/draw/nema_gfx/lv_draw_nema_gfx_utils.c',
	'lvgl/src/draw/nema_gfx/lv_nema_gfx_path.c',
	'lvgl/src/draw/nxp/g2d/lv_draw_buf_g2d.c',
	'lvgl/src/draw/nxp/g2d/lv_draw_g2d.c',
	'lvgl/src/draw/nxp/g2d/lv_draw_g2d_fill.c',
	'lvgl/src/draw/nxp/g2d/lv_draw_g2d_img.c',
	'lvgl/src/draw/nxp/g2d/lv_g2d_buf_map.c',
	'lvgl/src/draw/nxp/g2d/lv_g2d_utils.c',
	'lvgl/src/draw/nxp/pxp/lv_draw_buf_pxp.c',
	'lvgl/src/draw/nxp/pxp/lv_draw_pxp.c',
	'lvgl/src/draw/nxp/pxp/lv_draw_pxp_fill.c',
	'lvgl/src/draw/nxp/pxp/lv_draw_pxp_img.c',
	'lvgl/src/draw/nxp/pxp/lv_draw_pxp_layer.c',
	'lvgl/src/draw/nxp/pxp/lv_pxp_cfg.c',
	'lvgl/src/draw/nxp/pxp/lv_pxp_osa.c',
	'lvgl/src/draw/nxp/pxp/lv_pxp_utils.c',
	'lvgl/src/draw/nxp/vglite/lv_draw_buf_vglite.c',
	'lvgl/src/draw/nxp/vglite/lv_draw_vglite_arc.c',
	'lvgl/src/draw/nxp/vglite/lv_draw_vglite_border.c',
	'lvgl/src/draw/nxp/vglite/lv_draw_vglite.c',
	'lvgl/src/draw/nxp/vglite/lv_draw_vglite_fill.c',
	'lvgl/src/draw/nxp/vglite/lv_draw_vglite_img.c',
	'lvgl/src/draw/nxp/vglite/lv_draw_vglite_label.c',
	'lvgl/src/draw/nxp/vglite/lv_draw_vglite_layer.c',
	'lvgl/src/draw/nxp/vglite/lv_draw_vglite_line.c',
	'lvgl/src/draw/nxp/vglite/lv_draw_vglite_triangle.c',
	'lvgl/src/draw/nxp/vglite/lv_vglite_buf.c',
	'lvgl/src/draw/nxp/vglite/lv_vglite_matrix.c',
	'lvgl/src/draw/nxp/vglite/lv_vglite_path.c',
	'lvgl/src/draw/nxp/vglite/lv_vglite_utils.c',
	'lvgl/src/draw/opengles/lv_draw_opengles.c',
	'lvgl/src/draw/renesas/dave2d/lv_draw_dave2d_arc.c',
	'lvgl/src/draw/renesas/dave2d/lv_draw_dave2d_border.c',
	'lvgl/src/draw/renesas/dave2d/lv_draw_dave2d.c',
	'lvgl/src/draw/renesas/dave2d/lv_draw_dave2d_fill.c',
	'lvgl/src/draw/renesas/dave2d/lv_draw_dave2d_image.c',
	'lvgl/src/draw/renesas/dave2d/lv_draw_dave2d_label.c',
	'lvgl/src/draw/renesas/dave2d/lv_draw_dave2d_line.c',
	'lvgl/src/draw/renesas/dave2d/lv_draw_dave2d_mask_rectangle.c',
	'lvgl/src/draw/renesas/dave2d/lv_draw_dave2d_triangle.c',
	'lvgl/src/draw/renesas/dave2d/lv_draw_dave2d_utils.c',
	'lvgl/src/draw/sdl/lv_draw_sdl.c',
	'lvgl/src/draw/sw/blend/helium/lv_blend_helium.S',
	'lvgl/src/draw/sw/blend/lv_draw_sw_blend.c',
	'lvgl/src/draw/sw/blend/lv_draw_sw_blend_to_al88.c',
	'lvgl/src/draw/sw/blend/lv_draw_sw_blend_to_argb8888.c',
	'lvgl/src/draw/sw/blend/lv_draw_sw_blend_to_argb8888_premultiplied.c',
	'lvgl/src/draw/sw/blend/lv_draw_sw_blend_to_i1.c',
	'lvgl/src/draw/sw/blend/lv_draw_sw_blend_to_l8.c',
	'lvgl/src/draw/sw/blend/lv_draw_sw_blend_to_rgb565.c',
	'lvgl/src/draw/sw/blend/lv_draw_sw_blend_to_rgb888.c',
	'lvgl/src/draw/sw/blend/neon/lv_blend_neon.S',
	'lvgl/src/draw/sw/lv_draw_sw_arc.c',
	'lvgl/src/draw/sw/lv_draw_sw_border.c',
	'lvgl/src/draw/sw/lv_draw_sw_box_shadow.c',
	'lvgl/src/draw/sw/lv_draw_sw.c',
	'lvgl/src/draw/sw/lv_draw_sw_fill.c',
	'lvgl/src/draw/sw/lv_draw_sw_grad.c',
	'lvgl/src/draw/sw/lv_draw_sw_img.c',
	'lvgl/src/draw/sw/lv_draw_sw_letter.c',
	'lvgl/src/draw/sw/lv_draw_sw_line.c',
	'lvgl/src/draw/sw/lv_draw_sw_mask.c',
	'lvgl/src/draw/sw/lv_draw_sw_mask_rect.c',
	'lvgl/src/draw/sw/lv_draw_sw_transform.c',
	'lvgl/src/draw/sw/lv_draw_sw_triangle.c',
	'lvgl/src/draw/sw/lv_draw_sw_utils.c',
	'lvgl/src/draw/sw/lv_draw_sw_vector.c',
	'lvgl/src/draw/vg_lite/lv_draw_buf_vg_lite.c',
	'lvgl/src/draw/vg_lite/lv_draw_vg_lite_arc.c',
	'lvgl/src/draw/vg_lite/lv_draw_vg_lite_border.c',
	'lvgl/src/draw/vg_lite/lv_draw_vg_lite_box_shadow.c',
	'lvgl/src/draw/vg_lite/lv_draw_vg_lite.c',
	'lvgl/src/draw/vg_lite/lv_draw_vg_lite_fill.c',
	'lvgl/src/draw/vg_lite/lv_draw_vg_lite_img.c',
	'lvgl/src/draw/vg_lite/lv_draw_vg_lite_label.c',
	'lvgl/src/draw/vg_lite/lv_draw_vg_lite_layer.c',
	'lvgl/src/draw/vg_lite/lv_draw_vg_lite_line.c',
	'lvgl/src/draw/vg_lite/lv_draw_vg_lite_mask_rect.c',
	'lvgl/src/draw/vg_lite/lv_draw_vg_lite_triangle.c',
	'lvgl/src/draw/vg_lite/lv_draw_vg_lite_vector.c',
	'lvgl/src/draw/vg_lite/lv_vg_lite_decoder.c',
	'lvgl/src/draw/vg_lite/lv_vg_lite_grad.c',
	'lvgl/src/draw/vg_lite/lv_vg_lite_math.c',
	'lvgl/src/draw/vg_lite/lv_vg_lite_path.c',
	'lvgl/src/draw/vg_lite/lv_vg_lite_pending.c',
	'lvgl/src/draw/vg_lite/lv_vg_lite_stroke.c',
	'lvgl/src/draw/vg_lite/lv_vg_lite_utils.c',
	'lvgl/src/drivers/display/drm/lv_linux_drm.c',
	'lvgl/src/drivers/display/fb/lv_linux_fbdev.c',
	'lvgl/src/drivers/display/ft81x/lv_ft81x.c',
	'lvgl/src/drivers/display/ili9341/lv_ili9341.c',
	'lvgl/src/drivers/display/lcd/lv_lcd_generic_mipi.c',
	'lvgl/src/drivers/display/renesas_glcdc/lv_renesas_glcdc.c',
	'lvgl/src/drivers/display/st7735/lv_st7735.c',
	'lvgl/src/drivers/display/st7789/lv_st7789.c',
	'lvgl/src/drivers/display/st7796/lv_st7796.c',
	'lvgl/src/drivers/display/st_ltdc/lv_st_ltdc.c',
	'lvgl/src/drivers/display/tft_espi/lv_tft_espi.cpp',
	'lvgl/src/drivers/evdev/lv_evdev.c',
	'lvgl/src/drivers/glfw/lv_glfw_window.c',
	'lvgl/src/drivers/glfw/lv_opengles_debug.c',
	'lvgl/src/drivers/glfw/lv_opengles_driver.c',
	'lvgl/src/drivers/glfw/lv_opengles_texture.c',
	'lvgl/src/drivers/libinput/lv_libinput.c',
	'lvgl/src/drivers/libinput/lv_xkb.c',
	'lvgl/src/drivers/nuttx/lv_nuttx_cache.c',
	'lvgl/src/drivers/nuttx/lv_nuttx_entry.c',
	'lvgl/src/drivers/nuttx/lv_nuttx_fbdev.c',
	'lvgl/src/drivers/nuttx/lv_nuttx_image_cache.c',
	'lvgl/src/drivers/nuttx/lv_nuttx_lcd.c',
	'lvgl/src/drivers/nuttx/lv_nuttx_libuv.c',
	'lvgl/src/drivers/nuttx/lv_nuttx_profiler.c',
	'lvgl/src/drivers/nuttx/lv_nuttx_touchscreen.c',
	'lvgl/src/drivers/qnx/lv_qnx.c',
	'lvgl/src/drivers/sdl/lv_sdl_keyboard.c',
	'lvgl/src/drivers/sdl/lv_sdl_mouse.c',
	'lvgl/src/drivers/sdl/lv_sdl_mousewheel.c',
	'lvgl/src/drivers/sdl/lv_sdl_window.c',
	'lvgl/src/drivers/uefi/lv_uefi_context.c',
	'lvgl/src/drivers/uefi/lv_uefi_display.c',
	'lvgl/src/drivers/uefi/lv_uefi_indev_keyboard.c',
	'lvgl/src/drivers/uefi/lv_uefi_indev_pointer.c',
	'lvgl/src/drivers/uefi/lv_uefi_indev_touch.c',
	'lvgl/src/drivers/uefi/lv_uefi_private.c',
	'lvgl/src/drivers/wayland/lv_wayland.c',
	'lvgl/src/drivers/wayland/lv_wayland_smm.c',
	'lvgl/src/drivers/windows/lv_windows_context.c',
	'lvgl/src/drivers/windows/lv_windows_display.c',
	'lvgl/src/drivers/windows/lv_windows_input.c',
	'lvgl/src/drivers/x11/lv_x11_display.c',
	'lvgl/src/drivers/x11/lv_x11_input.c',
	'lvgl/src/font/lv_binfont_loader.c',
	'lvgl/src/font/lv_font.c',
	'lvgl/src/font/lv_font_dejavu_16_persian_hebrew.c',
	'lvgl/src/font/lv_font_fmt_txt.c',
	'lvgl/src/font/lv_font_montserrat_10.c',
	'lvgl/src/font/lv_font_montserrat_12.c',
	'lvgl/src/font/lv_font_montserrat_14.c',
	'lvgl/src/font/lv_font_montserrat_16.c',
	'lvgl/src/font/lv_font_montserrat_18.c',
	'lvgl/src/font/lv_font_montserrat_20.c',
	'lvgl/src/font/lv_font_montserrat_22.c',
	'lvgl/src/font/lv_font_montserrat_24.c',
	'lvgl/src/font/lv_font_montserrat_26.c',
	'lvgl/src/font/lv_font_montserrat_28.c',
	'lvgl/src/font/lv_font_montserrat_28_compressed.c',
	'lvgl/src/font/lv_font_montserrat_30.c',
	'lvgl/src/font/lv_font_montserrat_32.c',
	'lvgl/src/font/lv_font_montserrat_34.c',
	'lvgl/src/font/lv_font_montserrat_36.c',
	'lvgl/src/font/lv_font_montserrat_38.c',
	'lvgl/src/font/lv_font_montserrat_40.c',
	'lvgl/src/font/lv_font_montserrat_42.c',
	'lvgl/src/font/lv_font_montserrat_44.c',
	'lvgl/src/font/lv_font_montserrat_46.c',
	'lvgl/src/font/lv_font_montserrat_48.c',
	'lvgl/src/font/lv_font_montserrat_8.c',
	'lvgl/src/font/lv_font_simsun_14_cjk.c',
	'lvgl/src/font/lv_font_simsun_16_cjk.c',
	'lvgl/src/font/lv_font_source_han_sans_sc_14_cjk.c',
	'lvgl/src/font/lv_font_source_han_sans_sc_16_cjk.c',
	'lvgl/src/font/lv_font_unscii_16.c',
	'lvgl/src/font/lv_font_unscii_8.c',
	'lvgl/src/indev/lv_indev.c',
	'lvgl/src/indev/lv_indev_gesture.c',
	'lvgl/src/indev/lv_indev_scroll.c',
	'lvgl/src/layouts/flex/lv_flex.c',
	'lvgl/src/layouts/grid/lv_grid.c',
	'lvgl/src/layouts/lv_layout.c',
	'lvgl/src/libs/barcode/code128.c',
	'lvgl/src/libs/barcode/lv_barcode.c',
	'lvgl/src/libs/bin_decoder/lv_bin_decoder.c',
	'lvgl/src/libs/bmp/lv_bmp.c',
	'lvgl/src/libs/expat/xmlparse.c',
	'lvgl/src/libs/expat/xmlrole.c',
	'lvgl/src/libs/expat/xmltok.c',
	'lvgl/src/libs/expat/xmltok_impl.c',
	'lvgl/src/libs/expat/xmltok_ns.c',
	'lvgl/src/libs/ffmpeg/lv_ffmpeg.c',
	'lvgl/src/libs/freetype/lv_freetype.c',
	'lvgl/src/libs/freetype/lv_freetype_glyph.c',
	'lvgl/src/libs/freetype/lv_freetype_image.c',
	'lvgl/src/libs/freetype/lv_freetype_outline.c',
	'lvgl/src/libs/freetype/lv_ftsystem.c',
	'lvgl/src/libs/fsdrv/lv_fs_arduino_esp_littlefs.cpp',
	'lvgl/src/libs/fsdrv/lv_fs_arduino_sd.cpp',
	'lvgl/src/libs/fsdrv/lv_fs_cbfs.c',
	'lvgl/src/libs/fsdrv/lv_fs_fatfs.c',
	'lvgl/src/libs/fsdrv/lv_fs_littlefs.c',
	'lvgl/src/libs/fsdrv/lv_fs_memfs.c',
	'lvgl/src/libs/fsdrv/lv_fs_posix.c',
	'lvgl/src/libs/fsdrv/lv_fs_stdio.c',
	'lvgl/src/libs/fsdrv/lv_fs_uefi.c',
	'lvgl/src/libs/fsdrv/lv_fs_win32.c',
	'lvgl/src/libs/gif/gifdec.c',
	'lvgl/src/libs/gif/lv_gif.c',
	'lvgl/src/libs/libjpeg_turbo/lv_libjpeg_turbo.c',
	'lvgl/src/libs/libpng/lv_libpng.c',
	'lvgl/src/libs/lodepng/lodepng.c',
	'lvgl/src/libs/lodepng/lv_lodepng.c',
	'lvgl/src/libs/lz4/lz4.c',
	'lvgl/src/libs/qrcode/lv_qrcode.c',
	'lvgl/src/libs/qrcode/qrcodegen.c',
	'lvgl/src/libs/rle/lv_rle.c',
	'lvgl/src/libs/rlottie/lv_rlottie.c',
	'lvgl/src/libs/svg/lv_svg.c',
	'lvgl/src/libs/svg/lv_svg_decoder.c',
	'lvgl/src/libs/svg/lv_svg_parser.c',
	'lvgl/src/libs/svg/lv_svg_render.c',
	'lvgl/src/libs/svg/lv_svg_token.c',
	'lvgl/src/libs/thorvg/tvgAccessor.cpp',
	'lvgl/src/libs/thorvg/tvgAnimation.cpp',
	'lvgl/src/libs/thorvg/tvgCanvas.cpp',
	'lvgl/src/libs/thorvg/tvgCapi.cpp',
	'lvgl/src/libs/thorvg/tvgCompressor.cpp',
	'lvgl/src/libs/thorvg/tvgFill.cpp',
	'lvgl/src/libs/thorvg/tvgGlCanvas.cpp',
	'lvgl/src/libs/thorvg/tvgInitializer.cpp',
	'lvgl/src/libs/thorvg/tvgLoader.cpp',
	'lvgl/src/libs/thorvg/tvgLottieAnimation.cpp',
	'lvgl/src/libs/thorvg/tvgLottieBuilder.cpp',
	'lvgl/src/libs/thorvg/tvgLottieExpressions.cpp',
	'lvgl/src/libs/thorvg/tvgLottieInterpolator.cpp',
	'lvgl/src/libs/thorvg/tvgLottieLoader.cpp',
	'lvgl/src/libs/thorvg/tvgLottieModel.cpp',
	'lvgl/src/libs/thorvg/tvgLottieModifier.cpp',
	'lvgl/src/libs/thorvg/tvgLottieParser.cpp',
	'lvgl/src/libs/thorvg/tvgLottieParserHandler.cpp',
	'lvgl/src/libs/thorvg/tvgMath.cpp',
	'lvgl/src/libs/thorvg/tvgPaint.cpp',
	'lvgl/src/libs/thorvg/tvgPicture.cpp',
	'lvgl/src/libs/thorvg/tvgRawLoader.cpp',
	'lvgl/src/libs/thorvg/tvgRender.cpp',
	'lvgl/src/libs/thorvg/tvgSaver.cpp',
	'lvgl/src/libs/thorvg/tvgScene.cpp',
	'lvgl/src/libs/thorvg/tvgShape.cpp',
	'lvgl/src/libs/thorvg/tvgStr.cpp',
	'lvgl/src/libs/thorvg/tvgSvgCssStyle.cpp',
	'lvgl/src/libs/thorvg/tvgSvgLoader.cpp',
	'lvgl/src/libs/thorvg/tvgSvgPath.cpp',
	'lvgl/src/libs/thorvg/tvgSvgSceneBuilder.cpp',
	'lvgl/src/libs/thorvg/tvgSvgUtil.cpp',
	'lvgl/src/libs/thorvg/tvgSwCanvas.cpp',
	'lvgl/src/libs/thorvg/tvgSwFill.cpp',
	'lvgl/src/libs/thorvg/tvgSwImage.cpp',
	'lvgl/src/libs/thorvg/tvgSwMath.cpp',
	'lvgl/src/libs/thorvg/tvgSwMemPool.cpp',
	'lvgl/src/libs/thorvg/tvgSwPostEffect.cpp',
	'lvgl/src/libs/thorvg/tvgSwRaster.cpp',
	'lvgl/src/libs/thorvg/tvgSwRenderer.cpp',
	'lvgl/src/libs/thorvg/tvgSwRle.cpp',
	'lvgl/src/libs/thorvg/tvgSwShape.cpp',
	'lvgl/src/libs/thorvg/tvgSwStroke.cpp',
	'lvgl/src/libs/thorvg/tvgTaskScheduler.cpp',
	'lvgl/src/libs/thorvg/tvgText.cpp',
	'lvgl/src/libs/thorvg/tvgWgCanvas.cpp',
	'lvgl/src/libs/thorvg/tvgXmlParser.cpp',
	'lvgl/src/libs/tiny_ttf/lv_tiny_ttf.c',
	'lvgl/src/libs/tjpgd/lv_tjpgd.c',
	'lvgl/src/libs/tjpgd/tjpgd.c',
	'lvgl/src/lv_init.c',
	'lvgl/src/misc/cache/class/lv_cache_lru_ll.c',
	'lvgl/src/misc/cache/class/lv_cache_lru_rb.c',
	'lvgl/src/misc/cache/instance/lv_image_cache.c',
	'lvgl/src/misc/cache/instance/lv_image_header_cache.c',
	'lvgl/src/misc/cache/lv_cache.c',
	'lvgl/src/misc/cache/lv_cache_entry.c',
	'lvgl/src/misc/lv_anim.c',
	'lvgl/src/misc/lv_anim_timeline.c',
	'lvgl/src/misc/lv_area.c',
	'lvgl/src/misc/lv_array.c',
	'lvgl/src/misc/lv_async.c',
	'lvgl/src/misc/lv_bidi.c',
	'lvgl/src/misc/lv_circle_buf.c',
	'lvgl/src/misc/lv_color.c',
	'lvgl/src/misc/lv_color_op.c',
	'lvgl/src/misc/lv_event.c',
	'lvgl/src/misc/lv_fs.c',
	'lvgl/src/misc/lv_grad.c',
	'lvgl/src/misc/lv_iter.c',
	'lvgl/src/misc/lv_ll.c',
	'lvgl/src/misc/lv_log.c',
	'lvgl/src/misc/lv_lru.c',
	'lvgl/src/misc/lv_math.c',
	'lvgl/src/misc/lv_matrix.c',
	'lvgl/src/misc/lv_palette.c',
	'lvgl/src/misc/lv_profiler_builtin.c',
	'lvgl/src/misc/lv_rb.c',
	'lvgl/src/misc/lv_style.c',
	'lvgl/src/misc/lv_style_gen.c',
	'lvgl/src/misc/lv_templ.c',
	'lvgl/src/misc/lv_text_ap.c',
	'lvgl/src/misc/lv_text.c',
	'lvgl/src/misc/lv_timer.c',
	'lvgl/src/misc/lv_tree.c',
	'lvgl/src/misc/lv_utils.c',
	'lvgl/src/osal/lv_cmsis_rtos2.c',
	'lvgl/src/osal/lv_freertos.c',
	'lvgl/src/osal/lv_linux.c',
	'lvgl/src/osal/lv_mqx.c',
	'lvgl/src/osal/lv_os.c',
	'lvgl/src/osal/lv_os_none.c',
	'lvgl/src/osal/lv_pthread.c',
	'lvgl/src/osal/lv_rtthread.c',
	'lvgl/src/osal/lv_sdl2.c',
	'lvgl/src/osal/lv_windows.c',
	'lvgl/src/others/file_explorer/lv_file_explorer.c',
	'lvgl/src/others/font_manager/lv_font_manager.c',
	'lvgl/src/others/font_manager/lv_font_manager_recycle.c',
	'lvgl/src/others/fragment/lv_fragment.c',
	'lvgl/src/others/fragment/lv_fragment_manager.c',
	'lvgl/src/others/gridnav/lv_gridnav.c',
	'lvgl/src/others/ime/lv_ime_pinyin.c',
	'lvgl/src/others/imgfont/lv_imgfont.c',
	'lvgl/src/others/monkey/lv_monkey.c',
	'lvgl/src/others/observer/lv_observer.c',
	'lvgl/src/others/snapshot/lv_snapshot.c',
	'lvgl/src/others/sysmon/lv_sysmon.c',
	'lvgl/src/others/test/lv_test_display.c',
	'lvgl/src/others/test/lv_test_helpers.c',
	'lvgl/src/others/test/lv_test_indev.c',
	'lvgl/src/others/test/lv_test_indev_gesture.c',
	'lvgl/src/others/test/lv_test_screenshot_compare.c',
	'lvgl/src/others/vg_lite_tvg/vg_lite_matrix.c',
	'lvgl/src/others/vg_lite_tvg/vg_lite_tvg.cpp',
	'lvgl/src/others/xml/lv_xml_base_types.c',
	'lvgl/src/others/xml/lv_xml.c',
	'lvgl/src/others/xml/lv_xml_component.c',
	'lvgl/src/others/xml/lv_xml_parser.c',
	'lvgl/src/others/xml/lv_xml_style.c',
	'lvgl/src/others/xml/lv_xml_utils.c',
	'lvgl/src/others/xml/lv_xml_widget.c',
	'lvgl/src/others/xml/parsers/lv_xml_arc_parser.c',
	'lvgl/src/others/xml/parsers/lv_xml_bar_parser.c',
	'lvgl/src/others/xml/parsers/lv_xml_buttonmatrix_parser.c',
	'lvgl/src/others/xml/parsers/lv_xml_button_parser.c',
	'lvgl/src/others/xml/parsers/lv_xml_chart_parser.c',
	'lvgl/src/others/xml/parsers/lv_xml_checkbox_parser.c',
	'lvgl/src/others/xml/parsers/lv_xml_dropdown_parser.c',
	'lvgl/src/others/xml/parsers/lv_xml_event_parser.c',
	'lvgl/src/others/xml/parsers/lv_xml_image_parser.c',
	'lvgl/src/others/xml/parsers/lv_xml_keyboard_parser.c',
	'lvgl/src/others/xml/parsers/lv_xml_label_parser.c',
	'lvgl/src/others/xml/parsers/lv_xml_obj_parser.c',
	'lvgl/src/others/xml/parsers/lv_xml_roller_parser.c',
	'lvgl/src/others/xml/parsers/lv_xml_scale_parser.c',
	'lvgl/src/others/xml/parsers/lv_xml_slider_parser.c',
	'lvgl/src/others/xml/parsers/lv_xml_spangroup_parser.c',
	'lvgl/src/others/xml/parsers/lv_xml_table_parser.c',
	'lvgl/src/others/xml/parsers/lv_xml_tabview_parser.c',
	'lvgl/src/others/xml/parsers/lv_xml_textarea_parser.c',
	'lvgl/src/stdlib/builtin/lv_mem_core_builtin.c',
	'lvgl/src/stdlib/builtin/lv_sprintf_builtin.c',
	'lvgl/src/stdlib/builtin/lv_string_builtin.c',
	'lvgl/src/stdlib/builtin/lv_tlsf.c',
	'lvgl/src/stdlib/clib/lv_mem_core_clib.c',
	'lvgl/src/stdlib/clib/lv_sprintf_clib.c',
	'lvgl/src/stdlib/clib/lv_string_clib.c',
	'lvgl/src/stdlib/lv_mem.c',
	'lvgl/src/stdlib/micropython/lv_mem_core_micropython.c',
	'lvgl/src/stdlib/rtthread/lv_mem_core_rtthread.c',
	'lvgl/src/stdlib/rtthread/lv_sprintf_rtthread.c',
	'lvgl/src/stdlib/rtthread/lv_string_rtthread.c',
	'lvgl/src/stdlib/uefi/lv_mem_core_uefi.c',
	'lvgl/src/themes/default/lv_theme_default.c',
	'lvgl/src/themes/lv_theme.c',
	'lvgl/src/themes/mono/lv_theme_mono.c',
	'lvgl/src/themes/simple/lv_theme_simple.c',
	'lvgl/src/tick/lv_tick.c',
	'lvgl/src/widgets/3dtexture/lv_3dtexture.c',
	'lvgl/src/widgets/animimage/lv_animimage.c',
	'lvgl/src/widgets/arc/lv_arc.c',
	'lvgl/src/widgets/bar/lv_bar.c',
	'lvgl/src/widgets/button/lv_button.c',
	'lvgl/src/widgets/buttonmatrix/lv_buttonmatrix.c',
	'lvgl/src/widgets/calendar/lv_calendar.c',
	'lvgl/src/widgets/calendar/lv_calendar_chinese.c',
	'lvgl/src/widgets/calendar/lv_calendar_header_arrow.c',
	'lvgl/src/widgets/calendar/lv_calendar_header_dropdown.c',
	'lvgl/src/widgets/canvas/lv_canvas.c',
	'lvgl/src/widgets/chart/lv_chart.c',
	'lvgl/src/widgets/checkbox/lv_checkbox.c',
	'lvgl/src/widgets/dropdown/lv_dropdown.c',
	'lvgl/src/widgets/imagebutton/lv_imagebutton.c',
	'lvgl/src/widgets/image/lv_image.c',
	'lvgl/src/widgets/keyboard/lv_keyboard.c',
	'lvgl/src/widgets/label/lv_label.c',
	'lvgl/src/widgets/led/lv_led.c',
	'lvgl/src/widgets/line/lv_line.c',
	'lvgl/src/widgets/list/lv_list.c',
	'lvgl/src/widgets/lottie/lv_lottie.c',
	'lvgl/src/widgets/menu/lv_menu.c',
	'lvgl/src/widgets/msgbox/lv_msgbox.c',
	'lvgl/src/widgets/objx_templ/lv_objx_templ.c',
	'lvgl/src/widgets/property/lv_animimage_properties.c',
	'lvgl/src/widgets/property/lv_dropdown_properties.c',
	'lvgl/src/widgets/property/lv_image_properties.c',
	'lvgl/src/widgets/property/lv_keyboard_properties.c',
	'lvgl/src/widgets/property/lv_label_properties.c',
	'lvgl/src/widgets/property/lv_obj_properties.c',
	'lvgl/src/widgets/property/lv_roller_properties.c',
	'lvgl/src/widgets/property/lv_slider_properties.c',
	'lvgl/src/widgets/property/lv_style_properties.c',
	'lvgl/src/widgets/property/lv_textarea_properties.c',
	'lvgl/src/widgets/roller/lv_roller.c',
	'lvgl/src/widgets/scale/lv_scale.c',
	'lvgl/src/widgets/slider/lv_slider.c',
	'lvgl/src/widgets/span/lv_span.c',
	'lvgl/src/widgets/spinbox/lv_spinbox.c',
	'lvgl/src/widgets/spinner/lv_spinner.c',
	'lvgl/src/widgets/switch/lv_switch.c',
	'lvgl/src/widgets/table/lv_table.c',
	'lvgl/src/widgets/tabview/lv_tabview.c',
	'lvgl/src/widgets/textarea/lv_textarea.c',
	'lvgl/src/widgets/tileview/lv_tileview.c',
	'lvgl/src/widgets/win/lv_win.c',
])
lvgl_args = [
	'-DLV_CONF_INCLUDE_SIMPLE',
]
nanodistro_inc += include_directories('lvgl')
liblvgl = static_library(
	'lvgl',
	lvgl_srcs,
	include_directories: nanodistro_inc,
	c_args: nanodistro_args + nanodistro_c_args + lvgl_args,
	cpp_args: nanodistro_args + nanodistro_cpp_args + lvgl_args,
	dependencies: [libpng, libjpeg, libfreetype2, libdrm],
)
nanodistro_comps += liblvgl
libtsm_srcs = files([
	'libtsm/src/shared/shl-htable.c',
        'libtsm/src/shared/shl-pty.c',
        'libtsm/src/shared/shl-ring.c',
        'libtsm/src/tsm/tsm-vte.c',
        'libtsm/src/tsm/tsm-render.c',
        'libtsm/src/tsm/tsm-vte-charsets.c',
        'libtsm/src/tsm/tsm-selection.c',
        'libtsm/src/tsm/tsm-unicode.c',
        'libtsm/src/tsm/tsm-screen.c',
        'libtsm/external/wcwidth/wcwidth.c',
])
libtsm_args = [
	'-Wno-old-style-declaration',
	'-Wno-unused-variable',
	'-Wno-pedantic',
	'-D_GNU_SOURCE',
]
nanodistro_inc += include_directories('libtsm/src/tsm')
nanodistro_inc += include_directories('libtsm/src/shared')
nanodistro_inc += include_directories('libtsm/external')
libtsm = static_library(
	'libtsm',
	libtsm_srcs,
	include_directories: nanodistro_inc,
	c_args: nanodistro_args + nanodistro_c_args + libtsm_args,
	cpp_args: nanodistro_args + nanodistro_cpp_args + libtsm_args,
)
nanodistro_comps += libtsm
