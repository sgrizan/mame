//============================================================
//
//  d3dhlsl.c - Win32 Direct3D HLSL implementation
//
//============================================================
//
//  Copyright Aaron Giles
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the
//  following conditions are met:
//
//    * Redistributions of source code must retain the above
//      copyright notice, this list of conditions and the
//      following disclaimer.
//    * Redistributions in binary form must reproduce the
//      above copyright notice, this list of conditions and
//      the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//    * Neither the name 'MAME' nor the names of its
//      contributors may be used to endorse or promote
//      products derived from this software without specific
//      prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
//  EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGE (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
//  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//============================================================

// Useful info:
//  Windows XP/2003 shipped with DirectX 8.1
//  Windows 2000 shipped with DirectX 7a
//  Windows 98SE shipped with DirectX 6.1a
//  Windows 98 shipped with DirectX 5
//  Windows NT shipped with DirectX 3.0a
//  Windows 95 shipped with DirectX 2

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <math.h>
#undef interface

// MAME headers
#include "emu.h"
#include "render.h"
#include "ui.h"
#include "rendutil.h"
#include "options.h"
#include "emuopts.h"
#include "aviio.h"
#include "png.h"
#include "screen.h"

// MAMEOS headers
#include "d3dintf.h"
#include "winmain.h"
#include "window.h"
#include "config.h"
#include "d3dcomm.h"
#include "drawd3d.h"



//============================================================
//  GLOBALS
//============================================================

static slider_state *g_slider_list;
static file_error open_next(d3d::renderer *d3d, emu_file &file, const char *templ, const char *extension, int idx);

namespace d3d
{
hlsl_options shaders::s_hlsl_presets[4] =
{
	{   // 25% Shadow mask, 50% Scanlines, 3% Pincushion, 0 defocus, No Tint, 0.9 Exponent, 5% Floor, 25% Phosphor Return, 120% Saturation
		true,
		0.25f, { "aperture.png" }, 320, 240, 0.09375f, 0.109375f,
		0.03f, 0.03f,
		0.5f, 1.0f, 0.5f, 1.0f, 0.0f, 0.0f,
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 1.0f },
		{ 0.9f, 0.9f, 0.9f },
		{ 0.05f,0.05f,0.05f},
		{ 0.25f,0.25f,0.25f},
		1.2f,
		false, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0,
		0.9f, 4.0f,
		1.0f, 0.21f, 0.19f, 0.17f, 0.15f, 0.14f, 0.13f, 0.12f, 0.11f, 0.10f, 0.09f
	},
	{   // 25% Shadow mask, 0% Scanlines, 3% Pincushion, 0 defocus, No Tint, 0.9 Exponent, 5% Floor, 25% Phosphor Return, 120% Saturation
		true,
		0.25f, { "aperture.png" }, 320, 240, 0.09375f, 0.109375f,
		0.03f, 0.03f,
		0.0f, 1.0f, 0.5f, 1.0f, 0.0f, 0.0f,
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 1.0f },
		{ 0.9f, 0.9f, 0.9f },
		{ 0.05f,0.05f,0.05f},
		{ 0.25f,0.25f,0.25f},
		1.2f,
		false, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0,
		0.9f, 4.0f,
		1.0f, 0.21f, 0.19f, 0.17f, 0.15f, 0.14f, 0.13f, 0.12f, 0.11f, 0.10f, 0.09f
	},
	{   // 25% Shadow mask, 0% Scanlines, 0% Pincushion, 0 defocus, No Tint, 0.9 Exponent, 5% Floor, 25% Phosphor Return, 120% Saturation
		true,
		0.25f, { "aperture.png" }, 320, 240, 0.09375f, 0.109375f,
		0.0f, 0.0f,
		0.0f, 1.0f, 0.5f, 1.0f, 0.0f, 0.0f,
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 1.0f },
		{ 0.9f, 0.9f, 0.9f },
		{ 0.05f,0.05f,0.05f},
		{ 0.25f,0.25f,0.25f},
		1.2f,
		false, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0,
		0.9f, 4.0f,
		1.0f, 0.21f, 0.19f, 0.17f, 0.15f, 0.14f, 0.13f, 0.12f, 0.11f, 0.10f, 0.09f
	},
	{   // 25% Shadow mask, 100% Scanlines, 15% Pincushion, 3 defocus, 24-degree Tint Out, 1.5 Exponent, 5% Floor, 70% Phosphor Return, 80% Saturation, Bad Convergence
		true,
		0.25f, { "aperture.png" }, 320, 240, 0.09375f, 0.109375f,
		0.15f, 0.15f,
		1.0f, 1.0f, 0.5f, 1.0f, 0.0f, 0.5f,
		{ 3.0f, 3.0f, 3.0f, 3.0f },
		{ 0.5f,-0.33f,0.7f },
		{ 0.0f,-1.0f, 0.5f },
		{ 0.0f, 0.2f, 0.3f },
		{ 0.0f, 0.2f, 0.0f },
		{ 0.8f, 0.2f, 0.0f },
		{ 0.0f, 0.8f, 0.2f},
		{ 0.2f, 0.0f, 0.8f},
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 1.0f },
		{ 1.5f, 1.5f, 1.5f },
		{ 0.05f,0.05f,0.05f},
		{ 0.7f, 0.7f, 0.7f},
		0.8f,
		false, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0,
		0.9f, 4.0f,
		1.0f, 0.21f, 0.19f, 0.17f, 0.15f, 0.14f, 0.13f, 0.12f, 0.11f, 0.10f, 0.09f
	},
};

//============================================================
//  PROTOTYPES
//============================================================

static void get_vector(const char *data, int count, float *out, int report_error);



//============================================================
//  shader manager constructor
//============================================================

shaders::shaders()
{
	master_enable = false;
	vector_enable = true;
	prescale_size_x = 1;
	prescale_size_y = 1;
	prescale_force_x = 0;
	prescale_force_y = 0;
	preset = -1;
	shadow_texture = NULL;
	options = NULL;
	paused = true;
	lastidx = -1;
	targethead = NULL;
	cachehead = NULL;
	initialized = false;
}



//============================================================
//  shaders destructor
//============================================================

shaders::~shaders()
{
	global_free(options);
	cache_target *currcache = cachehead;
	while(cachehead != NULL)
	{
		cachehead = currcache->next;
		global_free(currcache);
		currcache = cachehead;
	}

	render_target *currtarget = targethead;
	while(targethead != NULL)
	{
		targethead = currtarget->next;
		global_free(currtarget);
		currtarget = targethead;
	}
}



//============================================================
//  shaders::window_save
//============================================================

void shaders::window_save()
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	renderer *d3d = (renderer *)window->drawdata;

	HRESULT result = (*d3dintf->device.create_texture)(d3d->get_device(), snap_width, snap_height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &snap_copy_texture);
	if (result != D3D_OK)
	{
		mame_printf_verbose("Direct3D: Unable to init system-memory target for HLSL snapshot (%08x), bailing\n", (UINT32)result);
		return;
	}
	(*d3dintf->texture.get_surface_level)(snap_copy_texture, 0, &snap_copy_target);

	result = (*d3dintf->device.create_texture)(d3d->get_device(), snap_width, snap_height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &snap_texture);
	if (result != D3D_OK)
	{
		mame_printf_verbose("Direct3D: Unable to init video-memory target for HLSL snapshot (%08x), bailing\n", (UINT32)result);
		return;
	}
	(*d3dintf->texture.get_surface_level)(snap_texture, 0, &snap_target);

	render_snap = true;
	snap_rendered = false;
}



//============================================================
//  shaders::window_record
//============================================================

void shaders::window_record()
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	windows_options &options = downcast<windows_options &>(window->machine().options());
	const char *filename = options.d3d_hlsl_write();

	if (avi_output_file != NULL)
		end_avi_recording();
	else if (filename[0] != 0)
		begin_avi_recording(filename);
}


//============================================================
//  shaders::avi_update_snap
//============================================================

void shaders::avi_update_snap(surface *surface)
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	renderer *d3d = (renderer *)window->drawdata;

	D3DLOCKED_RECT rect;

	// if we don't have a bitmap, or if it's not the right size, allocate a new one
	if (!avi_snap.valid() || (int)snap_width != avi_snap.width() || (int)snap_height != avi_snap.height())
	{
		avi_snap.allocate((int)snap_width, (int)snap_height);
	}

	// copy the texture
	HRESULT result = (*d3dintf->device.get_render_target_data)(d3d->get_device(), surface, avi_copy_surface);
	if (result != D3D_OK)
	{
		return;
	}

	// lock the texture
	result = (*d3dintf->surface.lock_rect)(avi_copy_surface, &rect, NULL, D3DLOCK_DISCARD);
	if (result != D3D_OK)
	{
		return;
	}

	// loop over Y
	for (int srcy = 0; srcy < (int)snap_height; srcy++)
	{
		DWORD *src = (DWORD *)((BYTE *)rect.pBits + srcy * rect.Pitch);
		UINT32 *dst = &avi_snap.pix32(srcy);

		for(int x = 0; x < snap_width; x++)
			*dst++ = *src++;
	}

	// unlock
	result = (*d3dintf->surface.unlock_rect)(avi_copy_surface);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during texture unlock_rect call\n", (int)result);
}



//============================================================
//  hlsl_render_snapshot
//============================================================

void shaders::render_snapshot(surface *surface)
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	renderer *d3d = (renderer *)window->drawdata;

	D3DLOCKED_RECT rect;

	render_snap = false;

	// if we don't have a bitmap, or if it's not the right size, allocate a new one
	if (!avi_snap.valid() || snap_width != (avi_snap.width() / 2) || snap_height != (avi_snap.height() / 2))
	{
		avi_snap.allocate(snap_width / 2, snap_height / 2);
	}

	// copy the texture
	HRESULT result = (*d3dintf->device.get_render_target_data)(d3d->get_device(), surface, snap_copy_target);
	if (result != D3D_OK)
	{
		return;
	}

	// lock the texture
	result = (*d3dintf->surface.lock_rect)(snap_copy_target, &rect, NULL, D3DLOCK_DISCARD);
	if (result != D3D_OK)
	{
		return;
	}

	for(int cy = 0; cy < 2; cy++)
	{
		for(int cx = 0; cx < 2; cx++)
		{
			// loop over Y
			for (int srcy = 0; srcy < snap_height / 2; srcy++)
			{
				int toty = (srcy + cy * (snap_height / 2));
				int totx = cx * (snap_width / 2);
				DWORD *src = (DWORD *)((BYTE *)rect.pBits + toty * rect.Pitch + totx * 4);
				UINT32 *dst = &avi_snap.pix32(srcy);

				for(int x = 0; x < snap_width / 2; x++)
					*dst++ = *src++;
			}

			int idx = cy * 2 + cx;

			emu_file file(window->machine().options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
			file_error filerr = open_next(d3d, file, NULL, "png", idx);
			if (filerr != FILERR_NONE)
				return;

			// add two text entries describing the image
			astring text1(emulator_info::get_appname(), " ", build_version);
			astring text2(window->machine().system().manufacturer, " ", window->machine().system().description);
			png_info pnginfo = { 0 };
			png_add_text(&pnginfo, "Software", text1);
			png_add_text(&pnginfo, "System", text2);

			// now do the actual work
			png_error error = png_write_bitmap(file, &pnginfo, avi_snap, 1 << 24, NULL);
			if (error != PNGERR_NONE)
				mame_printf_error("Error generating PNG for HLSL snapshot: png_error = %d\n", error);

			// free any data allocated
			png_free(&pnginfo);
		}
	}

	// unlock
	result = (*d3dintf->surface.unlock_rect)(snap_copy_target);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during texture unlock_rect call\n", (int)result);

	if(snap_texture != NULL)
	{
		(*d3dintf->texture.release)(snap_texture);
		snap_texture = NULL;
	}

	if(snap_target != NULL)
	{
		(*d3dintf->surface.release)(snap_target);
		snap_target = NULL;
	}

	if(snap_copy_texture != NULL)
	{
		(*d3dintf->texture.release)(snap_copy_texture);
		snap_copy_texture = NULL;
	}

	if(snap_copy_target != NULL)
	{
		(*d3dintf->surface.release)(snap_copy_target);
		snap_copy_target = NULL;
	}
}


//============================================================
//  shaders::record_texture
//============================================================

void shaders::record_texture()
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	surface *surface = avi_final_target;

	// ignore if nothing to do
	if (avi_output_file == NULL || surface == NULL)
		return;

	// get the current time
	attotime curtime = window->machine().time();

	avi_update_snap(surface);

	// loop until we hit the right time
	while (avi_next_frame_time <= curtime)
	{
		// handle an AVI recording
		// write the next frame
		avi_error avierr = avi_append_video_frame(avi_output_file, avi_snap);
		if (avierr != AVIERR_NONE)
		{
			end_avi_recording();
			return;
		}

		// advance time
		avi_next_frame_time += avi_frame_period;
		avi_frame++;
	}
}


//============================================================
//  shaders::end_hlsl_avi_recording
//============================================================

void shaders::end_avi_recording()
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	if (avi_output_file != NULL)
		avi_close(avi_output_file);

	avi_output_file = NULL;
	avi_frame = 0;
}


//============================================================
//  shaders::toggle
//============================================================

void shaders::toggle()
{
	if (master_enable)
	{
		if (initialized)
		{
			delete_resources(false);
		}
		master_enable = !master_enable;
	}
	else
	{
		if (!initialized)
		{
			master_enable = !master_enable;
			bool failed = create_resources(false);
			if (failed)
			{
				master_enable = false;
			}
		}
		else
		{
			master_enable = !master_enable;
		}
	}
}

//============================================================
//  shaders::begin_avi_recording
//============================================================

void shaders::begin_avi_recording(const char *name)
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	renderer *d3d = (renderer *)window->drawdata;

	// stop any existing recording
	end_avi_recording();

	// reset the state
	avi_frame = 0;
	avi_next_frame_time = window->machine().time();

	// build up information about this new movie
	avi_movie_info info;
	info.video_format = 0;
	info.video_timescale = 1000 * ((window->machine().primary_screen != NULL) ? ATTOSECONDS_TO_HZ(window->machine().primary_screen->frame_period().attoseconds) : screen_device::DEFAULT_FRAME_RATE);
	info.video_sampletime = 1000;
	info.video_numsamples = 0;
	info.video_width = snap_width;
	info.video_height = snap_height;
	info.video_depth = 24;

	info.audio_format = 0;
	info.audio_timescale = window->machine().sample_rate();
	info.audio_sampletime = 1;
	info.audio_numsamples = 0;
	info.audio_channels = 2;
	info.audio_samplebits = 16;
	info.audio_samplerate = window->machine().sample_rate();

	// create a new temporary movie file
	file_error filerr;
	astring fullpath;
	{
		emu_file tempfile(window->machine().options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
		if (name != NULL)
		{
			filerr = tempfile.open(name);
		}
		else
		{
			filerr = open_next(d3d, tempfile, NULL, "avi", 0);
		}

		// compute the frame time
		{
			avi_frame_period = attotime::from_seconds(1000) / info.video_timescale;
		}

		// if we succeeded, make a copy of the name and create the real file over top
		if (filerr == FILERR_NONE)
		{
			fullpath = tempfile.fullpath();
		}
	}

	if (filerr == FILERR_NONE)
	{
		// create the file and free the string
		avi_error avierr = avi_create(fullpath, &info, &avi_output_file);
		if (avierr != AVIERR_NONE)
		{
			mame_printf_error("Error creating AVI: %s\n", avi_error_string(avierr));
		}
	}
}


//============================================================
//  remove_cache_target - remove an active cache target when
//  refcount hits zero
//============================================================

void shaders::remove_cache_target(cache_target *cache)
{
	if (cache != NULL)
	{
		if (cache == cachehead)
		{
			cachehead = cachehead->next;
		}

		if (cache->prev != NULL)
		{
			cache->prev->next = cache->next;
		}

		if (cache->next != NULL)
		{
			cache->next->prev = cache->prev;
		}

		global_free(cache);
	}
}


//============================================================
//  remove_render_target - remove an active target
//============================================================

void shaders::remove_render_target(texture_info *texture)
{
	remove_render_target(find_render_target(texture));
}

void shaders::remove_render_target(int width, int height, UINT32 screen_index, UINT32 page_index)
{
	render_target *target = find_render_target(width, height, screen_index, page_index);
	if (target != NULL)
	{
		remove_render_target(target);
	}
}

void shaders::remove_render_target(render_target *rt)
{
	if (rt != NULL)
	{
		if (rt == targethead)
		{
			targethead = targethead->next;
		}

		if (rt->prev != NULL)
		{
			rt->prev->next = rt->next;
		}

		if (rt->next != NULL)
		{
			rt->next->prev = rt->prev;
		}

		cache_target *cache = find_cache_target(rt->screen_index, rt->width, rt->height);
		if (cache != NULL)
		{
			remove_cache_target(cache);
		}

		int screen_index = rt->screen_index;
		int other_page = 1 - rt->page_index;
		int width = rt->width;
		int height = rt->height;

		global_free(rt);

		// Remove other double-buffered page (if it exists)
		remove_render_target(width, height, screen_index, other_page);
	}
}


//============================================================
//  shaders::set_texture
//============================================================

void shaders::set_texture(texture_info *texture)
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	renderer *d3d = (renderer *)window->drawdata;

	if(texture != NULL)
	{
		paused = texture->paused();
		texture->advance_frame();
	}

	texture_info *default_texture = d3d->get_default_texture();
	default_effect->set_texture("Diffuse", (texture == NULL) ? default_texture->get_finaltex() : texture->get_finaltex());
	if (options->yiq_enable)
		yiq_encode_effect->set_texture("Diffuse", (texture == NULL) ? default_texture->get_finaltex() : texture->get_finaltex());
	else
		color_effect->set_texture("Diffuse", (texture == NULL) ? default_texture->get_finaltex() : texture->get_finaltex());
	pincushion_effect->set_texture("Diffuse", (texture == NULL) ? default_texture->get_finaltex() : texture->get_finaltex());
}


//============================================================
//  shaders::init
//============================================================

void shaders::init(base *d3dintf, win_window_info *window)
{
	if (!d3dintf->post_fx_available)
		return;

	this->d3dintf = d3dintf;
	this->window = window;

	master_enable = downcast<windows_options &>(window->machine().options()).d3d_hlsl_enable();
	prescale_size_x = 1;
	prescale_size_y = 1;
	preset = downcast<windows_options &>(window->machine().options()).d3d_hlsl_preset();
	if (preset < -1 || preset > 3)
	{
		preset = -1;
	}

	snap_width = downcast<windows_options &>(window->machine().options()).d3d_snap_width();
	snap_height = downcast<windows_options &>(window->machine().options()).d3d_snap_height();
	prescale_force_x = 0;
	prescale_force_y = 0;

	windows_options &winoptions = downcast<windows_options &>(window->machine().options());

	options = (hlsl_options*)global_alloc_clear(hlsl_options);

	options->params_dirty = true;
	strcpy(options->shadow_mask_texture, downcast<windows_options &>(window->machine().options()).screen_shadow_mask_texture()); // unsafe

	prescale_force_x = winoptions.d3d_hlsl_prescale_x();
	prescale_force_y = winoptions.d3d_hlsl_prescale_y();
	if(preset == -1)
	{
		options->shadow_mask_alpha = winoptions.screen_shadow_mask_alpha();
		options->shadow_mask_count_x = winoptions.screen_shadow_mask_count_x();
		options->shadow_mask_count_y = winoptions.screen_shadow_mask_count_y();
		options->shadow_mask_u_size = winoptions.screen_shadow_mask_u_size();
		options->shadow_mask_v_size = winoptions.screen_shadow_mask_v_size();
		options->curvature = winoptions.screen_curvature();
		options->pincushion = winoptions.screen_pincushion();
		options->scanline_alpha = winoptions.screen_scanline_amount();
		options->scanline_scale = winoptions.screen_scanline_scale();
		options->scanline_height = winoptions.screen_scanline_height();
		options->scanline_bright_scale = winoptions.screen_scanline_bright_scale();
		options->scanline_bright_offset = winoptions.screen_scanline_bright_offset();
		options->scanline_offset = winoptions.screen_scanline_offset();
		get_vector(winoptions.screen_defocus(), 2, options->defocus, TRUE);
		get_vector(winoptions.screen_converge_x(), 3, options->converge_x, TRUE);
		get_vector(winoptions.screen_converge_y(), 3, options->converge_y, TRUE);
		get_vector(winoptions.screen_radial_converge_x(), 3, options->radial_converge_x, TRUE);
		get_vector(winoptions.screen_radial_converge_y(), 3, options->radial_converge_y, TRUE);
		get_vector(winoptions.screen_red_ratio(), 3, options->red_ratio, TRUE);
		get_vector(winoptions.screen_grn_ratio(), 3, options->grn_ratio, TRUE);
		get_vector(winoptions.screen_blu_ratio(), 3, options->blu_ratio, TRUE);
		get_vector(winoptions.screen_offset(), 3, options->offset, TRUE);
		get_vector(winoptions.screen_scale(), 3, options->scale, TRUE);
		get_vector(winoptions.screen_power(), 3, options->power, TRUE);
		get_vector(winoptions.screen_floor(), 3, options->floor, TRUE);
		get_vector(winoptions.screen_phosphor(), 3, options->phosphor, TRUE);
		options->saturation = winoptions.screen_saturation();
	}
	else
	{
		options = &s_hlsl_presets[preset];
	}

	options->yiq_enable = winoptions.screen_yiq_enable();
	options->yiq_cc = winoptions.screen_yiq_cc();
	options->yiq_a = winoptions.screen_yiq_a();
	options->yiq_b = winoptions.screen_yiq_b();
	options->yiq_o = winoptions.screen_yiq_o();
	options->yiq_p = winoptions.screen_yiq_p();
	options->yiq_n = winoptions.screen_yiq_n();
	options->yiq_y = winoptions.screen_yiq_y();
	options->yiq_i = winoptions.screen_yiq_i();
	options->yiq_q = winoptions.screen_yiq_q();
	options->yiq_scan_time = winoptions.screen_yiq_scan_time();
	options->yiq_phase_count = winoptions.screen_yiq_phase_count();
	options->vector_length_scale = winoptions.screen_vector_length_scale();
	options->vector_length_ratio = winoptions.screen_vector_length_ratio();
	options->vector_bloom_scale = winoptions.screen_vector_bloom_scale();
	options->raster_bloom_scale = winoptions.screen_raster_bloom_scale();
	options->bloom_level0_weight = winoptions.screen_bloom_lvl0_weight();
	options->bloom_level1_weight = winoptions.screen_bloom_lvl1_weight();
	options->bloom_level2_weight = winoptions.screen_bloom_lvl2_weight();
	options->bloom_level3_weight = winoptions.screen_bloom_lvl3_weight();
	options->bloom_level4_weight = winoptions.screen_bloom_lvl4_weight();
	options->bloom_level5_weight = winoptions.screen_bloom_lvl5_weight();
	options->bloom_level6_weight = winoptions.screen_bloom_lvl6_weight();
	options->bloom_level7_weight = winoptions.screen_bloom_lvl7_weight();
	options->bloom_level8_weight = winoptions.screen_bloom_lvl8_weight();
	options->bloom_level9_weight = winoptions.screen_bloom_lvl9_weight();
	options->bloom_level10_weight = winoptions.screen_bloom_lvl10_weight();

	options->params_dirty = true;

	g_slider_list = init_slider_list();
}



//============================================================
//  shaders::init_fsfx_quad
//============================================================

void shaders::init_fsfx_quad(void *vertbuf)
{
	// Called at the start of each frame by the D3D code in order to reserve two triangles
	// that are guaranteed to be at a fixed position so as to simply use D3DPT_TRIANGLELIST, 0, 2
	// instead of having to do bookkeeping about a specific screen quad
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	renderer *d3d = (renderer *)window->drawdata;

	// get a pointer to the vertex buffer
	fsfx_vertices = (vertex *)vertbuf;
	if (fsfx_vertices == NULL)
		return;

	// fill in the vertexes clockwise
	fsfx_vertices[0].x = 0.0f;
	fsfx_vertices[0].y = 0.0f;
	fsfx_vertices[1].x = d3d->get_width();
	fsfx_vertices[1].y = 0.0f;
	fsfx_vertices[2].x = 0.0f;
	fsfx_vertices[2].y = d3d->get_height();
	fsfx_vertices[3].x = d3d->get_width();
	fsfx_vertices[3].y = 0.0f;
	fsfx_vertices[4].x = 0.0f;
	fsfx_vertices[4].y = d3d->get_height();
	fsfx_vertices[5].x = d3d->get_width();
	fsfx_vertices[5].y = d3d->get_height();

	fsfx_vertices[0].u0 = 0.0f;
	fsfx_vertices[0].v0 = 0.0f;

	fsfx_vertices[1].u0 = 1.0f;
	fsfx_vertices[1].v0 = 0.0f;

	fsfx_vertices[2].u0 = 0.0f;
	fsfx_vertices[2].v0 = 1.0f;

	fsfx_vertices[3].u0 = 1.0f;
	fsfx_vertices[3].v0 = 0.0f;

	fsfx_vertices[4].u0 = 0.0f;
	fsfx_vertices[4].v0 = 1.0f;

	fsfx_vertices[5].u0 = 1.0f;
	fsfx_vertices[5].v0 = 1.0f;

	fsfx_vertices[0].u1 = 0.0f;
	fsfx_vertices[0].v1 = 0.0f;
	fsfx_vertices[1].u1 = 0.0f;
	fsfx_vertices[1].v1 = 0.0f;
	fsfx_vertices[2].u1 = 0.0f;
	fsfx_vertices[2].v1 = 0.0f;
	fsfx_vertices[3].u1 = 0.0f;
	fsfx_vertices[3].v1 = 0.0f;
	fsfx_vertices[4].u1 = 0.0f;
	fsfx_vertices[4].v1 = 0.0f;
	fsfx_vertices[5].u1 = 0.0f;
	fsfx_vertices[5].v1 = 0.0f;

	// set the color, Z parameters to standard values
	for (int i = 0; i < 6; i++)
	{
		fsfx_vertices[i].z = 0.0f;
		fsfx_vertices[i].rhw = 1.0f;
		fsfx_vertices[i].color = D3DCOLOR_ARGB(255, 255, 255, 255);
	}
}



//============================================================
//  shaders::create_resources
//============================================================

int shaders::create_resources(bool reset)
{
	if (!master_enable || !d3dintf->post_fx_available)
		return 0;

	renderer *d3d = (renderer *)window->drawdata;

	HRESULT result = (*d3dintf->device.get_render_target)(d3d->get_device(), 0, &backbuffer);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device get_render_target call\n", (int)result);

	result = (*d3dintf->device.create_texture)(d3d->get_device(), 4, 4, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &black_texture);
	if (result != D3D_OK)
	{
		mame_printf_verbose("Direct3D: Unable to init video-memory target for black texture (%08x)\n", (UINT32)result);
		return 1;
	}
	(*d3dintf->texture.get_surface_level)(black_texture, 0, &black_surface);
	result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, black_surface);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);
	result = (*d3dintf->device.clear)(d3d->get_device(), 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);
	result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, backbuffer);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);

	result = (*d3dintf->device.create_texture)(d3d->get_device(), (int)snap_width, (int)snap_height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &avi_copy_texture);
	if (result != D3D_OK)
	{
		mame_printf_verbose("Direct3D: Unable to init system-memory target for HLSL AVI dumping (%08x)\n", (UINT32)result);
		return 1;
	}
	(*d3dintf->texture.get_surface_level)(avi_copy_texture, 0, &avi_copy_surface);

	result = (*d3dintf->device.create_texture)(d3d->get_device(), (int)snap_width, (int)snap_height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &avi_final_texture);
	if (result != D3D_OK)
	{
		mame_printf_verbose("Direct3D: Unable to init video-memory target for HLSL AVI dumping (%08x)\n", (UINT32)result);
		return 1;
	}
	(*d3dintf->texture.get_surface_level)(avi_final_texture, 0, &avi_final_target);

	emu_file file(window->machine().options().art_path(), OPEN_FLAG_READ);
	render_load_png(shadow_bitmap, file, NULL, options->shadow_mask_texture);

	// experimental: if we have a shadow bitmap, create a texture for it
	if (shadow_bitmap.valid())
	{
		render_texinfo texture;

		// fake in the basic data so it looks like it came from render.c
		texture.base = shadow_bitmap.raw_pixptr(0);
		texture.rowpixels = shadow_bitmap.rowpixels();
		texture.width = shadow_bitmap.width();
		texture.height = shadow_bitmap.height();
		texture.palette = NULL;
		texture.seqid = 0;

		// now create it
		shadow_texture = new texture_info(d3d->get_texture_manager(), &texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32));
	}

	const char *fx_dir = downcast<windows_options &>(window->machine().options()).screen_post_fx_dir();

	default_effect = new effect(d3d->get_device(), "primary.fx", fx_dir);
	post_effect = new effect(d3d->get_device(), "post.fx", fx_dir);
	prescale_effect = new effect(d3d->get_device(), "prescale.fx", fx_dir);
	pincushion_effect = new effect(d3d->get_device(), "pincushion.fx", fx_dir);
	phosphor_effect = new effect(d3d->get_device(), "phosphor.fx", fx_dir);
	focus_effect = new effect(d3d->get_device(), "focus.fx", fx_dir);
	deconverge_effect = new effect(d3d->get_device(), "deconverge.fx", fx_dir);
	color_effect = new effect(d3d->get_device(), "color.fx", fx_dir);
	yiq_encode_effect = new effect(d3d->get_device(), "yiq_encode.fx", fx_dir);
	yiq_decode_effect = new effect(d3d->get_device(), "yiq_decode.fx", fx_dir);
	bloom_effect = new effect(d3d->get_device(), "bloom.fx", fx_dir);
	downsample_effect = new effect(d3d->get_device(), "downsample.fx", fx_dir);
	vector_effect = new effect(d3d->get_device(), "vector.fx", fx_dir);

	if (!default_effect->is_valid()) return 1;
	if (!post_effect->is_valid()) return 1;
	if (!prescale_effect->is_valid()) return 1;
	if (!pincushion_effect->is_valid()) return 1;
	if (!phosphor_effect->is_valid()) return 1;
	if (!focus_effect->is_valid()) return 1;
	if (!deconverge_effect->is_valid()) return 1;
	if (!color_effect->is_valid()) return 1;
	if (!yiq_encode_effect->is_valid()) return 1;
	if (!yiq_decode_effect->is_valid()) return 1;
	if (!bloom_effect->is_valid()) return 1;
	if (!downsample_effect->is_valid()) return 1;
	if (!vector_effect->is_valid()) return 1;

	initialized = true;

	return 0;
}


//============================================================
//  shaders::begin_draw
//============================================================

void shaders::begin_draw()
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	renderer *d3d = (renderer *)window->drawdata;

	curr_effect = default_effect;

	default_effect->set_technique("TestTechnique");
	post_effect->set_technique("ScanMaskTechnique");
	pincushion_effect->set_technique("TestTechnique");
	phosphor_effect->set_technique("TestTechnique");
	focus_effect->set_technique("TestTechnique");
	deconverge_effect->set_technique("DeconvergeTechnique");
	color_effect->set_technique("ColorTechnique");
	yiq_encode_effect->set_technique("EncodeTechnique");
	yiq_decode_effect->set_technique("DecodeTechnique");

	HRESULT result = (*d3dintf->device.get_render_target)(d3d->get_device(), 0, &backbuffer);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device get_render_target call\n", (int)result);
}


//============================================================
//  shaders::begin_frame
//============================================================

void shaders::begin_frame()
{
	record_texture();
}


//============================================================
//  shaders::blit
//============================================================

void shaders::blit(surface *dst, texture *src, surface *new_dst, D3DPRIMITIVETYPE prim_type,
						UINT32 prim_index, UINT32 prim_count, int dstw, int dsth)
{
	renderer *d3d = (renderer *)window->drawdata;

	HRESULT result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, dst);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);

	curr_effect = default_effect;

	curr_effect->set_texture("Diffuse", src);

	curr_effect->set_float("TargetWidth", (float)dstw);
	curr_effect->set_float("TargetHeight", (float)dsth);
	curr_effect->set_float("PostPass", 1.0f);
	curr_effect->set_float("PincushionAmount", options->pincushion);
	curr_effect->set_float("Brighten", 0.0f);

	unsigned int num_passes = 0;
	curr_effect->begin(&num_passes, 0);

	for (UINT pass = 0; pass < num_passes; pass++)
	{
		curr_effect->begin_pass(pass);
		// add the primitives
		HRESULT result = (*d3dintf->device.draw_primitive)(d3d->get_device(), prim_type, prim_index, prim_count);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
		curr_effect->end_pass();
	}

	curr_effect->end();

	if (new_dst)
	{
		HRESULT result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, new_dst);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);
	}
}



//============================================================
//  shaders::blit
//============================================================

void shaders::blit(surface *dst, texture *src, surface *new_dst, D3DPRIMITIVETYPE prim_type,
						UINT32 prim_index, UINT32 prim_count)
{
	renderer *d3d = (renderer *)window->drawdata;

	HRESULT result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, dst);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);
	result = (*d3dintf->device.clear)(d3d->get_device(), 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(1,0,0,0), 0, 0);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

	curr_effect = default_effect;

	curr_effect->set_texture("Diffuse", src);

	curr_effect->set_float("TargetWidth", (float)d3d->get_width());
	curr_effect->set_float("TargetHeight", (float)d3d->get_height());
	curr_effect->set_float("ScreenWidth", (float)d3d->get_width());
	curr_effect->set_float("ScreenHeight", (float)d3d->get_height());
	curr_effect->set_float("PostPass", 1.0f);
	curr_effect->set_float("PincushionAmount", options->pincushion);
	curr_effect->set_float("Brighten", 1.0f);

	unsigned int num_passes = 0;
	curr_effect->begin(&num_passes, 0);

	for (UINT pass = 0; pass < num_passes; pass++)
	{
		curr_effect->begin_pass(pass);
		// add the primitives
		HRESULT result = (*d3dintf->device.draw_primitive)(d3d->get_device(), prim_type, prim_index, prim_count);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
		curr_effect->end_pass();
	}

	curr_effect->end();

	curr_effect->set_float("Brighten", 0.0f);

	if (new_dst)
	{
		HRESULT result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, new_dst);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);
	}
}

//============================================================
//  shaders::end_frame
//============================================================

void shaders::end_frame()
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	if(render_snap && snap_rendered)
	{
		render_snapshot(snap_target);
	}

	if (!lines_pending)
		return;

	lines_pending = false;

	/*render_target *rt = find_render_target(d3d->get_width(), d3d->get_height(), 0, 0);
	if (rt == NULL)
	{
	    return;
	}

	blit(backbuffer, rt->render_texture[1], NULL, vecbuf_type, vecbuf_index, vecbuf_count);

	HRESULT result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, rt->target[1]);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);
	result = (*d3dintf->device.clear)(d3d->get_device(), 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);
	result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, backbuffer);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);*/
}


//============================================================
//  shaders::init_effect_info
//============================================================

void shaders::init_effect_info(poly_info *poly)
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	renderer *d3d = (renderer *)window->drawdata;
	texture_info *texture = poly->get_texture();

	vec2f shadow_dims;

	if (shadow_texture)
	{
		shadow_dims = shadow_texture->get_rawdims();
	}
	else
	{
		shadow_dims.c.x = 1.0f;
		shadow_dims.c.y = 1.0f;
	}

	if(PRIMFLAG_GET_TEXSHADE(d3d->get_last_texture_flags()))
	{
		curr_effect = pincushion_effect;
	}
	else if(PRIMFLAG_GET_SCREENTEX(d3d->get_last_texture_flags()) && texture != NULL)
	{
		// Plug in all of the shader settings we're going to need
		// This is extremely slow, but we're not rendering models here,
		// just post-processing.
		curr_effect = post_effect;

		curr_effect->set_float("ScanlineOffset", (texture->get_cur_frame() == 0) ? 0.0f : options->scanline_offset);

		if(options->params_dirty)
		{
			vec2f delta = texture->get_uvstop() - texture->get_uvstart();
			curr_effect->set_vector("RawDims", 2, &texture->get_rawdims().c.x);
			curr_effect->set_vector("SizeRatio", 2, &delta.c.x);
			curr_effect->set_float("TargetWidth", d3d->get_width());
			curr_effect->set_float("TargetHeight", d3d->get_height());
			curr_effect->set_vector("Floor", 3, options->floor);
			curr_effect->set_float("SnapX", snap_width);
			curr_effect->set_float("SnapY", snap_height);
			curr_effect->set_float("PincushionAmount", options->pincushion);
			curr_effect->set_float("CurvatureAmount", options->curvature);
			curr_effect->set_float("UseShadow", shadow_texture == NULL ? 0.0f : 1.0f);
			curr_effect->set_texture("Shadow", shadow_texture == NULL ? NULL : shadow_texture->get_finaltex());
			curr_effect->set_float("ShadowBrightness", options->shadow_mask_alpha);
			curr_effect->set_float("ShadowMaskSizeX", (float)options->shadow_mask_count_x);
			curr_effect->set_float("ShadowMaskSizeY", (float)options->shadow_mask_count_y);
			curr_effect->set_float("ShadowU", options->shadow_mask_u_size);
			curr_effect->set_float("ShadowV", options->shadow_mask_v_size);

			curr_effect->set_vector("ShadowDims", 2, &shadow_dims.c.x);
			curr_effect->set_float("ScanlineAmount", options->scanline_alpha);
			curr_effect->set_float("ScanlineScale", options->scanline_scale);
			curr_effect->set_float("ScanlineHeight", options->scanline_height);
			curr_effect->set_float("ScanlineBrightScale", options->scanline_bright_scale);
			curr_effect->set_float("ScanlineBrightOffset", options->scanline_bright_offset);
			//curr_effect->set_float("ScanlineOffset", (texture->get_cur_frame() == 0) ? 0.0f : options->scanline_offset);
			curr_effect->set_vector("Power", 3, options->power);
		}
	}
	else
	{
		curr_effect = default_effect;

		curr_effect->set_float("FixedAlpha", 1.0f);
	}
}


//============================================================
//  shaders::find_render_target
//============================================================

render_target* shaders::find_render_target(texture_info *info)
{
	render_target *curr = targethead;
	UINT32 screen_index_data = (UINT32)info->get_texinfo().osddata;
	UINT32 screen_index = screen_index_data >> 1;
	UINT32 page_index = screen_index_data & 1;

	while (curr != NULL && (curr->screen_index != screen_index || curr->page_index != page_index ||
		curr->width != info->get_texinfo().width || curr->height != info->get_texinfo().height))
	{
		curr = curr->next;
	}

	return curr;
}


//============================================================
//  shaders::find_render_target
//============================================================

render_target* shaders::find_render_target(int width, int height, UINT32 screen_index, UINT32 page_index)
{
	render_target *curr = targethead;

	while (curr != NULL && (curr->width != width || curr->height != height || curr->screen_index != screen_index || curr->page_index != page_index))
	{
		curr = curr->next;
	}

	return curr;
}


//============================================================
//  shaders::find_cache_target
//============================================================

cache_target* shaders::find_cache_target(UINT32 screen_index, int width, int height)
{
	cache_target *curr = cachehead;

	while (curr != NULL && (curr->screen_index != screen_index || curr->width != width || curr->height != height))
	{
		curr = curr->next;
	}

	return curr;
}

void shaders::ntsc_pass(render_target *rt, texture_info *texture, vec2f &texsize, vec2f &delta)
{
	renderer *d3d = (renderer *)window->drawdata;
	UINT num_passes = 0;

	if(options->yiq_enable)
	{
		// Convert our signal into YIQ
		curr_effect = yiq_encode_effect;

		if(options->params_dirty)
		{
			curr_effect->set_vector("RawDims", 2, &texsize.c.x);
			curr_effect->set_float("WidthRatio", 1.0f / delta.c.x);
			curr_effect->set_float("HeightRatio", 1.0f / delta.c.y);
			curr_effect->set_float("ScreenWidth", d3d->get_width());
			curr_effect->set_float("ScreenHeight", d3d->get_height());
			curr_effect->set_float("CCValue", options->yiq_cc);
			curr_effect->set_float("AValue", options->yiq_a);
			curr_effect->set_float("BValue", options->yiq_b);
			curr_effect->set_float("PValue", options->yiq_p);
			curr_effect->set_float("NotchHalfWidth", options->yiq_n);
			curr_effect->set_float("YFreqResponse", options->yiq_y);
			curr_effect->set_float("IFreqResponse", options->yiq_i);
			curr_effect->set_float("QFreqResponse", options->yiq_q);
			curr_effect->set_float("ScanTime", options->yiq_scan_time);
		}

		HRESULT result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, rt->target[4]);

		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);
		result = (*d3dintf->device.clear)(d3d->get_device(), 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

		curr_effect->begin(&num_passes, 0);

		for (UINT pass = 0; pass < num_passes; pass++)
		{
			curr_effect->begin_pass(pass);
			// add the primitives
			result = (*d3dintf->device.draw_primitive)(d3d->get_device(), D3DPT_TRIANGLELIST, 0, 2);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
			curr_effect->end_pass();
		}

		curr_effect->end();

		// Convert our signal from YIQ
		curr_effect = yiq_decode_effect;

		curr_effect->set_texture("Composite", rt->render_texture[4]);
		curr_effect->set_texture("Diffuse", texture->get_finaltex());
		if(true)//options->params_dirty)
		{
			curr_effect->set_vector("RawDims", 2, &texsize.c.x);
			curr_effect->set_float("WidthRatio", 1.0f / delta.c.x);
			curr_effect->set_float("HeightRatio", 1.0f / delta.c.y);
			curr_effect->set_float("ScreenWidth", d3d->get_width());
			curr_effect->set_float("ScreenHeight", d3d->get_height());
			curr_effect->set_float("CCValue", options->yiq_cc);
			curr_effect->set_float("AValue", options->yiq_a);
			curr_effect->set_float("BValue", options->yiq_b);
			curr_effect->set_float("OValue", options->yiq_o);
			curr_effect->set_float("PValue", options->yiq_p);
			curr_effect->set_float("NotchHalfWidth", options->yiq_n);
			curr_effect->set_float("YFreqResponse", options->yiq_y);
			curr_effect->set_float("IFreqResponse", options->yiq_i);
			curr_effect->set_float("QFreqResponse", options->yiq_q);
			curr_effect->set_float("ScanTime", options->yiq_scan_time);
		}

		result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, rt->target[3]);

		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);
		result = (*d3dintf->device.clear)(d3d->get_device(), 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

		curr_effect->begin(&num_passes, 0);

		for (UINT pass = 0; pass < num_passes; pass++)
		{
			curr_effect->begin_pass(pass);
			// add the primitives
			result = (*d3dintf->device.draw_primitive)(d3d->get_device(), D3DPT_TRIANGLELIST, 0, 2);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
			curr_effect->end_pass();
		}

		curr_effect->end();

		curr_effect = color_effect;

		curr_effect->set_texture("Diffuse", rt->render_texture[3]);
	}
}

void shaders::color_convolution_pass(render_target *rt, texture_info *texture, vec2f &texsize, vec2f &delta, vec2f &rawdims)
{
	renderer *d3d = (renderer *)window->drawdata;
	UINT num_passes = 0;

	curr_effect = color_effect;

	// Render the initial color-convolution pass
	if(options->params_dirty)
	{
		curr_effect->set_vector("RawDims", 2, &rawdims.c.x);
		curr_effect->set_float("TargetWidth", (float)d3d->get_width());
		curr_effect->set_float("TargetHeight", (float)d3d->get_height());
		curr_effect->set_float("YIQEnable", options->yiq_enable ? 1.0f : 0.0f);
		curr_effect->set_vector("RedRatios", 3, options->red_ratio);
		curr_effect->set_vector("GrnRatios", 3, options->grn_ratio);
		curr_effect->set_vector("BluRatios", 3, options->blu_ratio);
		curr_effect->set_vector("Offset", 3, options->offset);
		curr_effect->set_vector("Scale", 3, options->scale);
		curr_effect->set_float("Saturation", options->saturation);
	}

	HRESULT result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, rt->smalltarget);

	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);
	result = (*d3dintf->device.clear)(d3d->get_device(), 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

	curr_effect->begin(&num_passes, 0);

	for (UINT pass = 0; pass < num_passes; pass++)
	{
		curr_effect->begin_pass(pass);
		// add the primitives
		result = (*d3dintf->device.draw_primitive)(d3d->get_device(), D3DPT_TRIANGLELIST, 0, 2);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
		curr_effect->end_pass();
	}

	curr_effect->end();
}

void shaders::prescale_pass(render_target *rt, texture_info *texture, vec2f &texsize, vec2f &delta, vec2f &rawdims)
{
	renderer *d3d = (renderer *)window->drawdata;
	UINT num_passes = 0;

	curr_effect = prescale_effect;
	curr_effect->set_texture("Diffuse", rt->smalltexture);

	if(options->params_dirty)
	{
		curr_effect->set_float("TargetWidth", (float)d3d->get_width());
		curr_effect->set_float("TargetHeight", (float)d3d->get_height());
		curr_effect->set_vector("RawDims", 2, &rawdims.c.x);
	}

	curr_effect->begin(&num_passes, 0);

	HRESULT result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, rt->prescaletarget);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);
	result = (*d3dintf->device.clear)(d3d->get_device(), 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

	for (UINT pass = 0; pass < num_passes; pass++)
	{
		curr_effect->begin_pass(pass);
		// add the primitives
		result = (*d3dintf->device.draw_primitive)(d3d->get_device(), D3DPT_TRIANGLELIST, 0, 2);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
		curr_effect->end_pass();
	}

	curr_effect->end();
}

void shaders::deconverge_pass(render_target *rt, texture_info *texture, vec2f &texsize, vec2f &delta, vec2f &rawdims)
{
	renderer *d3d = (renderer *)window->drawdata;
	UINT num_passes = 0;

	curr_effect = deconverge_effect;
	curr_effect->set_texture("Diffuse", rt->prescaletexture);

	if(options->params_dirty)
	{
		curr_effect->set_float("TargetWidth", (float)d3d->get_width());
		curr_effect->set_float("TargetHeight", (float)d3d->get_height());
		curr_effect->set_vector("RawDims", 2, &rawdims.c.x);
		curr_effect->set_vector("SizeRatio", 2, &delta.c.x);
		curr_effect->set_vector("ConvergeX", 3, options->converge_x);
		curr_effect->set_vector("ConvergeY", 3, options->converge_y);
		curr_effect->set_vector("RadialConvergeX", 3, options->radial_converge_x);
		curr_effect->set_vector("RadialConvergeY", 3, options->radial_converge_y);
	}

	curr_effect->begin(&num_passes, 0);

	HRESULT result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, rt->target[2]);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call 6\n", (int)result);
	result = (*d3dintf->device.clear)(d3d->get_device(), 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

	for (UINT pass = 0; pass < num_passes; pass++)
	{
		curr_effect->begin_pass(pass);
		// add the primitives
		result = (*d3dintf->device.draw_primitive)(d3d->get_device(), D3DPT_TRIANGLELIST, 0, 2);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
		curr_effect->end_pass();
	}

	curr_effect->end();
}

void shaders::defocus_pass(render_target *rt, texture_info *texture, vec2f &texsize, vec2f &delta, vec2f &rawdims)
{
	renderer *d3d = (renderer *)window->drawdata;
	UINT num_passes = 0;

	// Defocus pass 1
	curr_effect = focus_effect;

	curr_effect->set_texture("Diffuse", rt->render_texture[2]);

	curr_effect->set_float("TargetWidth", (float)d3d->get_width());
	curr_effect->set_float("TargetHeight", (float)d3d->get_height());
	curr_effect->set_vector("Defocus", 2, &options->defocus[0]);

	curr_effect->begin(&num_passes, 0);

	HRESULT result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, rt->target[0]);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call 6\n", (int)result);
	result = (*d3dintf->device.clear)(d3d->get_device(), 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

	for (UINT pass = 0; pass < num_passes; pass++)
	{
		curr_effect->begin_pass(pass);
		// add the primitives
		result = (*d3dintf->device.draw_primitive)(d3d->get_device(), D3DPT_TRIANGLELIST, 0, 2);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
		curr_effect->end_pass();
	}

	curr_effect->end();

	// Defocus pass 2

	curr_effect->set_texture("Diffuse", rt->render_texture[0]);

	curr_effect->set_float("TargetWidth", (float)d3d->get_width());
	curr_effect->set_float("TargetHeight", (float)d3d->get_height());
	curr_effect->set_vector("Defocus", 2, &options->defocus[1]);

	curr_effect->begin(&num_passes, 0);

	result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, rt->target[1]);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call 7\n", (int)result);

	for (UINT pass = 0; pass < num_passes; pass++)
	{
		curr_effect->begin_pass(pass);
		// add the primitives
		result = (*d3dintf->device.draw_primitive)(d3d->get_device(), D3DPT_TRIANGLELIST, 0, 2);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
		curr_effect->end_pass();
	}

	curr_effect->end();
}

void shaders::phosphor_pass(render_target *rt, cache_target *ct, texture_info *texture, vec2f &texsize, vec2f &delta, vec2f &rawdims, bool focus_enable)
{
	renderer *d3d = (renderer *)window->drawdata;
	UINT num_passes = 0;

	curr_effect = phosphor_effect;

	if(options->params_dirty)
	{
		curr_effect->set_float("TargetWidth", (float)d3d->get_width());
		curr_effect->set_float("TargetHeight", (float)d3d->get_height());
		curr_effect->set_vector("Phosphor", 3, options->phosphor);
	}
	curr_effect->set_float("TextureWidth", (float)rt->target_width);
	curr_effect->set_float("TextureHeight", (float)rt->target_height);
	curr_effect->set_float("Passthrough", 0.0f);

	curr_effect->set_texture("Diffuse", focus_enable ? rt->render_texture[1] : rt->render_texture[2]);
	curr_effect->set_texture("LastPass", ct->last_texture);

	HRESULT result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, rt->target[0]);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call 4\n", (int)result);
	result = (*d3dintf->device.clear)(d3d->get_device(), 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

	curr_effect->begin(&num_passes, 0);

	for (UINT pass = 0; pass < num_passes; pass++)
	{
		curr_effect->begin_pass(pass);
		// add the primitives
		result = (*d3dintf->device.draw_primitive)(d3d->get_device(), D3DPT_TRIANGLELIST, 0, 2);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
		curr_effect->end_pass();
	}

	curr_effect->end();

	// Pass along our phosphor'd screen
	curr_effect = phosphor_effect;

	curr_effect->set_texture("Diffuse", rt->render_texture[0]);
	curr_effect->set_texture("LastPass", rt->render_texture[0]);
	curr_effect->set_float("Passthrough", 1.0f);

	result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, ct->last_target); // Avoid changing targets due to page flipping
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call 5\n", (int)result);
	result = (*d3dintf->device.clear)(d3d->get_device(), 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

	curr_effect->begin(&num_passes, 0);

	for (UINT pass = 0; pass < num_passes; pass++)
	{
		curr_effect->begin_pass(pass);
		// add the primitives
		result = (*d3dintf->device.draw_primitive)(d3d->get_device(), D3DPT_TRIANGLELIST, 0, 2);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
		curr_effect->end_pass();
	}

	curr_effect->end();
}

void shaders::avi_post_pass(render_target *rt, texture_info *texture, vec2f &texsize, vec2f &delta, vec2f &rawdims, poly_info *poly, int vertnum)
{
	renderer *d3d = (renderer *)window->drawdata;
	UINT num_passes = 0;

	curr_effect = post_effect;
	curr_effect->set_float("TargetWidth", (float)d3d->get_width());
	curr_effect->set_float("TargetHeight", (float)d3d->get_height());

	// Scanlines and shadow mask, at high res for AVI logging
	if(avi_output_file != NULL)
	{
		curr_effect->set_texture("Diffuse", rt->render_texture[0]);

		HRESULT result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, avi_final_target);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);

		curr_effect->begin(&num_passes, 0);

		for (UINT pass = 0; pass < num_passes; pass++)
		{
			curr_effect->begin_pass(pass);
			// add the primitives
			result = (*d3dintf->device.draw_primitive)(d3d->get_device(), poly->get_type(), vertnum, poly->get_count());
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
			curr_effect->end_pass();
		}

		curr_effect->end();
	}

	if(render_snap)
	{
		curr_effect->set_texture("Diffuse", rt->render_texture[0]);

		HRESULT result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, snap_target);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);

		curr_effect->begin(&num_passes, 0);

		for (UINT pass = 0; pass < num_passes; pass++)
		{
			curr_effect->begin_pass(pass);
			// add the primitives
			result = (*d3dintf->device.draw_primitive)(d3d->get_device(), poly->get_type(), vertnum, poly->get_count());
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
			curr_effect->end_pass();
		}

		curr_effect->end();

		snap_rendered = true;
	}
}

void shaders::screen_post_pass(render_target *rt, texture_info *texture, vec2f &texsize, vec2f &delta, vec2f &rawdims, poly_info *poly, int vertnum)
{
	renderer *d3d = (renderer *)window->drawdata;
	UINT num_passes = 0;

	curr_effect = post_effect;

	curr_effect->set_texture("Diffuse", rt->render_texture[0]);
	curr_effect->set_vector("RawDims", 2, &rawdims.c.x);
	curr_effect->set_vector("SizeRatio", 2, &delta.c.x);

	d3d->set_wrap(D3DTADDRESS_MIRROR);

	curr_effect->set_float("TargetWidth", (float)d3d->get_width());
	curr_effect->set_float("TargetHeight", (float)d3d->get_height());

	HRESULT result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, rt->target[2]);

	d3d->set_wrap(D3DTADDRESS_MIRROR);
	result = (*d3dintf->device.clear)(d3d->get_device(), 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(1,0,0,0), 0, 0);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);
	curr_effect->set_float("TargetWidth", (float)d3d->get_width());
	curr_effect->set_float("TargetHeight", (float)d3d->get_height());

	curr_effect->begin(&num_passes, 0);

	for (UINT pass = 0; pass < num_passes; pass++)
	{
		curr_effect->begin_pass(pass);
		// add the primitives
		result = (*d3dintf->device.draw_primitive)(d3d->get_device(), poly->get_type(), vertnum, poly->get_count());
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
		curr_effect->end_pass();
	}

	curr_effect->end();

	d3d->set_wrap(PRIMFLAG_GET_TEXWRAP(poly->get_texture()->get_flags()) ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);
}

void shaders::raster_bloom_pass(render_target *rt, texture_info *texture, vec2f &texsize, vec2f &delta, vec2f &rawdims, poly_info *poly, int vertnum)
{
	renderer *d3d = (renderer *)window->drawdata;
	UINT num_passes = 0;

	curr_effect = downsample_effect;

	curr_effect->set_texture("Diffuse", rt->render_texture[2]);
	curr_effect->set_float("BloomRescale", options->raster_bloom_scale);

	float bloom_size = (d3d->get_width() < d3d->get_height()) ? d3d->get_width() : d3d->get_height();
	int bloom_index = 0;
	float bloom_width = rt->target_width;
	float bloom_height = rt->target_height;
	float prim_width = poly->get_prim_width();
	float prim_height = poly->get_prim_height();
	float prim_ratio[2] = { prim_width / bloom_width, prim_height / bloom_height };
	float screen_size[2] = { d3d->get_width(), d3d->get_height() };
	curr_effect->set_vector("ScreenSize", 2, screen_size);
	for(; bloom_size >= 2.0f && bloom_index < 11; bloom_size *= 0.5f)
	{
		float target_size[2] = { bloom_width, bloom_height };
		float source_size[2] = { bloom_width * 0.5f, bloom_height * 0.5f };
		curr_effect->set_vector("TargetSize", 2, target_size);
		curr_effect->set_vector("SourceSize", 2, source_size);
		curr_effect->set_vector("PrimRatio", 2, prim_ratio);

		curr_effect->begin(&num_passes, 0);

		curr_effect->set_texture("Diffuse", (bloom_index == 0) ? rt->render_texture[2] : rt->bloom_texture[bloom_index - 1]);

		HRESULT result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, rt->bloom_target[bloom_index]);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call 6\n", (int)result);

		for (UINT pass = 0; pass < num_passes; pass++)
		{
			curr_effect->begin_pass(pass);
			// add the primitives
			result = (*d3dintf->device.draw_primitive)(d3d->get_device(), D3DPT_TRIANGLELIST, 0, 2);
			//result = (*d3dintf->device.draw_primitive)(d3d->get_device(), poly->get_type(), vertnum, poly->get_count());
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
			curr_effect->end_pass();
		}

		curr_effect->end();

		bloom_index++;
		bloom_width *= 0.5f;
		bloom_height *= 0.5f;
	}

	curr_effect = bloom_effect;

	float target_size[2] = { d3d->get_width(), d3d->get_height() };
	curr_effect->set_vector("TargetSize", 2, target_size);
	float weight0123[4] = { options->bloom_level0_weight, options->bloom_level1_weight,
							options->bloom_level2_weight, options->bloom_level3_weight };
	float weight4567[4] = { options->bloom_level4_weight, options->bloom_level5_weight,
							options->bloom_level6_weight, options->bloom_level7_weight };
	float weight89A[3] = { options->bloom_level8_weight, options->bloom_level9_weight,
							options->bloom_level10_weight };
	curr_effect->set_vector("Level0123Weight", 4, weight0123);
	curr_effect->set_vector("Level4567Weight", 4, weight4567);
	curr_effect->set_vector("Level89AWeight", 3, weight89A);
	curr_effect->set_vector("TargetSize", 2, target_size);

	curr_effect->set_texture("DiffuseA", rt->render_texture[2]);
	curr_effect->set_float("DiffuseScaleA", 1.0f);

	char name[9] = "Diffuse*";
	char scale[14] = "DiffuseScale*";
	for(int index = 1; index < bloom_index; index++)
	{
		name[7] = 'A' + index;
		scale[12] = 'A' + index;
		curr_effect->set_texture(name, rt->bloom_texture[index - 1]);
		curr_effect->set_float(scale, 1.0f);
	}
	for(int index = bloom_index; index < 11; index++)
	{
		name[7] = 'A' + index;
		scale[12] = 'A' + index;
		curr_effect->set_texture(name, black_texture);
		curr_effect->set_float(scale, 0.0f);
	}

	curr_effect->begin(&num_passes, 0);

	HRESULT result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, backbuffer);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call 6\n", (int)result);

	for (UINT pass = 0; pass < num_passes; pass++)
	{
		curr_effect->begin_pass(pass);
		// add the primitives
		result = (*d3dintf->device.draw_primitive)(d3d->get_device(), poly->get_type(), vertnum, poly->get_count());
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
		curr_effect->end_pass();
	}

	curr_effect->end();
}

//============================================================
//  shaders::render_quad
//============================================================

void shaders::render_quad(poly_info *poly, int vertnum)
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	UINT num_passes = 0;
	renderer *d3d = (renderer *)window->drawdata;
	texture_info *texture = poly->get_texture();

	if(PRIMFLAG_GET_SCREENTEX(d3d->get_last_texture_flags()) && texture != NULL)
	{
		render_target *rt = find_render_target(texture);
		if (rt == NULL)
		{
			return;
		}
		cache_target *ct = find_cache_target(rt->screen_index, texture->get_texinfo().width, texture->get_texinfo().height);

		vec2f& rawdims = texture->get_rawdims();
		vec2f delta = texture->get_uvstop() - texture->get_uvstart();
		vec2f texsize(rt->width, rt->height);
		float defocus_x = options->defocus[0];
		float defocus_y = options->defocus[1];
		bool focus_enable = defocus_x != 0.0f || defocus_y != 0.0f;

		ntsc_pass(rt, texture, texsize, delta);
		color_convolution_pass(rt, texture, texsize, delta, rawdims);
		prescale_pass(rt, texture, texsize, delta, rawdims);
		deconverge_pass(rt, texture, texsize, delta, rawdims);
		if (focus_enable)
		{
			defocus_pass(rt, texture, texsize, delta, rawdims);
		}
		phosphor_pass(rt, ct, texture, texsize, delta, rawdims, focus_enable);
		avi_post_pass(rt, texture, texsize, delta, rawdims, poly, vertnum);
		screen_post_pass(rt, texture, texsize, delta, rawdims, poly, vertnum);
		raster_bloom_pass(rt, texture, texsize, delta, rawdims, poly, vertnum);

		texture->increment_frame_count();
		texture->mask_frame_count(options->yiq_phase_count);

		options->params_dirty = false;

	}
	else if(PRIMFLAG_GET_VECTOR(poly->get_flags()) && vector_enable)
	{
		render_target *rt = find_render_target(d3d->get_width(), d3d->get_height(), 0, 0);
		if (rt == NULL)
		{
			return;
		}

		lines_pending = true;

		curr_effect = vector_effect;

		if(options->params_dirty)
		{
			curr_effect->set_float("TargetWidth", (float)d3d->get_width());
			curr_effect->set_float("TargetHeight", (float)d3d->get_height());
		}

		float time_params[2] = { 0.0f, 0.0f };
		float length_params[3] = { poly->get_line_length(), options->vector_length_scale, options->vector_length_ratio };
		curr_effect->set_vector("TimeParams", 2, time_params);
		curr_effect->set_vector("LengthParams", 3, length_params);

		curr_effect->begin(&num_passes, 0);

		HRESULT result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, rt->target[0]);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);

		for (UINT pass = 0; pass < num_passes; pass++)
		{
			curr_effect->begin_pass(pass);
			// add the primitives
			HRESULT result = (*d3dintf->device.draw_primitive)(d3d->get_device(), poly->get_type(), vertnum, poly->get_count());
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
			curr_effect->end_pass();
		}

		curr_effect->end();
		result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, backbuffer);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);

		curr_effect = default_effect;

		curr_effect->set_float("FixedAlpha", 1.0f);
	}
	else if (PRIMFLAG_GET_VECTORBUF(poly->get_flags()) && vector_enable)
	{
		//if (!lines_pending)
			//return;
		//lines_pending = false;

		renderer *d3d = (renderer *)window->drawdata;

		render_target *rt = find_render_target(d3d->get_width(), d3d->get_height(), 0, 0);
		if (rt == NULL)
		{
			return;
		}

		/* Bloom */
		curr_effect = downsample_effect;

		curr_effect->set_texture("Diffuse", rt->render_texture[0]);
		curr_effect->set_float("BloomRescale", options->vector_bloom_scale);

		float bloom_size = (d3d->get_width() < d3d->get_height()) ? d3d->get_width() : d3d->get_height();
		int bloom_index = 0;
		float bloom_width = rt->target_width;
		float bloom_height = rt->target_height;
		float prim_width = poly->get_prim_width();
		float prim_height = poly->get_prim_height();
		float prim_ratio[2] = { prim_width / bloom_width, prim_height / bloom_height };
		float screen_size[2] = { d3d->get_width(), d3d->get_height() };
		//float target_size[2] = { bloom_width * 0.5f, bloom_height * 0.5f };
		curr_effect->set_vector("ScreenSize", 2, screen_size);
		for(; bloom_size >= 2.0f && bloom_index < 11; bloom_size *= 0.5f)
		{
			float target_size[2] = { bloom_width, bloom_height };
			float source_size[2] = { bloom_width * 0.5f, bloom_height * 0.5f };
			curr_effect->set_vector("TargetSize", 2, target_size);
			curr_effect->set_vector("SourceSize", 2, source_size);
			curr_effect->set_vector("PrimRatio", 2, prim_ratio);

			curr_effect->begin(&num_passes, 0);

			curr_effect->set_texture("Diffuse", (bloom_index == 0) ? rt->render_texture[0] : rt->bloom_texture[bloom_index - 1]);

			HRESULT result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, rt->bloom_target[bloom_index]);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call 6\n", (int)result);
			//result = (*d3dintf->device.clear)(d3d->get_device(), 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
			//if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

			for (UINT pass = 0; pass < num_passes; pass++)
			{
				curr_effect->begin_pass(pass);
				// add the primitives
				result = (*d3dintf->device.draw_primitive)(d3d->get_device(), poly->get_type(), vertnum, poly->get_count());
				if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
				curr_effect->end_pass();
			}

			curr_effect->end();

			bloom_index++;
			bloom_width *= 0.5f;
			bloom_height *= 0.5f;
		}

		// Bloom composite pass
		curr_effect = bloom_effect;

		float target_size[2] = { d3d->get_width(), d3d->get_height() };
		curr_effect->set_vector("TargetSize", 2, target_size);
		float weight0123[4] = { options->bloom_level0_weight, options->bloom_level1_weight,
								options->bloom_level2_weight, options->bloom_level3_weight };
		float weight4567[4] = { options->bloom_level4_weight, options->bloom_level5_weight,
								options->bloom_level6_weight, options->bloom_level7_weight };
		float weight89A[3] = { options->bloom_level8_weight, options->bloom_level9_weight,
								options->bloom_level10_weight };
		curr_effect->set_vector("Level0123Weight", 4, weight0123);
		curr_effect->set_vector("Level4567Weight", 4, weight4567);
		curr_effect->set_vector("Level89AWeight", 3, weight89A);

		curr_effect->set_texture("DiffuseA", rt->render_texture[0]);
		curr_effect->set_float("DiffuseScaleA", 1.0f);

		char name[9] = "Diffuse*";
		char scale[14] = "DiffuseScale*";
		for(int index = 1; index < bloom_index; index++)
		{
			name[7] = 'A' + index;
			scale[12] = 'A' + index;
			curr_effect->set_texture(name, rt->bloom_texture[index - 1]);
			curr_effect->set_float(scale, 1.0f);
		}
		for(int index = bloom_index; index < 11; index++)
		{
			name[7] = 'A' + index;
			scale[12] = 'A' + index;
			curr_effect->set_texture(name, black_texture);
			curr_effect->set_float(scale, 0.0f);
		}

		curr_effect->begin(&num_passes, 0);

		HRESULT result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, rt->target[1]);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call 6\n", (int)result);
		result = (*d3dintf->device.clear)(d3d->get_device(), 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

		for (UINT pass = 0; pass < num_passes; pass++)
		{
			curr_effect->begin_pass(pass);
			// add the primitives
			result = (*d3dintf->device.draw_primitive)(d3d->get_device(), poly->get_type(), vertnum, poly->get_count());
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
			curr_effect->end_pass();
		}

		curr_effect->end();

		/* Phosphor */
		curr_effect = phosphor_effect;

		if(options->params_dirty)
		{
			curr_effect->set_float("TargetWidth", (float)d3d->get_width());
			curr_effect->set_float("TargetHeight", (float)d3d->get_height());
			curr_effect->set_vector("Phosphor", 3, options->phosphor);
		}
		curr_effect->set_float("TextureWidth", (float)d3d->get_width());
		curr_effect->set_float("TextureHeight", (float)d3d->get_height());
		curr_effect->set_float("Passthrough", 0.0f);

		curr_effect->set_texture("Diffuse", rt->render_texture[1]);
		curr_effect->set_texture("LastPass", rt->render_texture[2]);

		result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, rt->target[3]);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call 4\n", (int)result);
		result = (*d3dintf->device.clear)(d3d->get_device(), 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

		curr_effect->begin(&num_passes, 0);

		for (UINT pass = 0; pass < num_passes; pass++)
		{
			curr_effect->begin_pass(pass);
			// add the primitives
			result = (*d3dintf->device.draw_primitive)(d3d->get_device(), D3DPT_TRIANGLELIST, 0, 2);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
			curr_effect->end_pass();
		}

		curr_effect->end();

		blit(rt->target[2], rt->render_texture[3], NULL, poly->get_type(), vertnum, poly->get_count());
		blit(backbuffer, rt->render_texture[3], backbuffer, poly->get_type(), vertnum, poly->get_count());

		result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, rt->target[0]);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);
		result = (*d3dintf->device.clear)(d3d->get_device(), 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);
		result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, backbuffer);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);

		lines_pending = false;
	}
	else
	{
		curr_effect = default_effect;

		curr_effect->set_float("TargetWidth", (float)d3d->get_width());
		curr_effect->set_float("TargetHeight", (float)d3d->get_height());
		curr_effect->set_float("PostPass", 0.0f);

		curr_effect->begin(&num_passes, 0);

		for (UINT pass = 0; pass < num_passes; pass++)
		{
			curr_effect->begin_pass(pass);
			// add the primitives
			HRESULT result = (*d3dintf->device.draw_primitive)(d3d->get_device(), poly->get_type(), vertnum, poly->get_count());
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
			curr_effect->end_pass();
		}

		curr_effect->end();
	}
}



//============================================================
//  shaders::end_draw
//============================================================

void shaders::end_draw()
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	(*d3dintf->surface.release)(backbuffer);
}


//============================================================
//  shaders::register_prescaled_texture
//============================================================

bool shaders::register_prescaled_texture(texture_info *texture)
{
	return register_texture(texture);
}


//============================================================
//  shaders::add_cache_target - register a cache target
//============================================================
bool shaders::add_cache_target(renderer* d3d, texture_info* info, int width, int height, int xprescale, int yprescale, int screen_index)
{
	printf("Adding cache target\n");
	cache_target* target = (cache_target*)global_alloc_clear(cache_target);

	if (!target->init(d3d, d3dintf, width, height, xprescale, yprescale))
	{
		global_free(target);
		return false;
	}

	if (info != NULL)
	{
		target->width = info->get_texinfo().width;
		target->height = info->get_texinfo().height;
	}
	else
	{
		target->width = d3d->get_width();
		target->height = d3d->get_height();
	}

	target->next = cachehead;
	target->prev = NULL;

	target->screen_index = screen_index;

	if (cachehead != NULL)
	{
		cachehead->prev = target;
	}
	cachehead = target;

	return true;
}

render_target* shaders::get_vector_target()
{
	if (!vector_enable)
	{
		return NULL;
	}

	renderer *d3d = (renderer *)window->drawdata;

	return find_render_target(d3d->get_width(), d3d->get_height(), 0, 0);
}

void shaders::create_vector_target(render_primitive *prim)
{
	renderer *d3d = (renderer *)window->drawdata;
	if (!add_render_target(d3d, NULL, d3d->get_width(), d3d->get_height(), 1, 1))
	{
		vector_enable = false;
	}
}

//============================================================
//  shaders::add_render_target - register a render target
//============================================================

bool shaders::add_render_target(renderer* d3d, texture_info* info, int width, int height, int xprescale, int yprescale)
{
	UINT32 screen_index = 0;
	UINT32 page_index = 0;
	if (info != NULL)
	{
		render_target *existing_target = find_render_target(info);
		if (existing_target != NULL)
		{
			remove_render_target(existing_target);
		}

		UINT32 screen_index_data = (UINT32)info->get_texinfo().osddata;
		screen_index = screen_index_data >> 1;
		page_index = screen_index_data & 1;
	}
	else
	{
		render_target *existing_target = find_render_target(d3d->get_width(), d3d->get_height(), 0, 0);
		if (existing_target != NULL)
		{
			remove_render_target(existing_target);
		}
	}

	render_target* target = (render_target*)global_alloc_clear(render_target);

	if (!target->init(d3d, d3dintf, width, height, xprescale, yprescale))
	{
		global_free(target);
		return false;
	}

	if (info != NULL)
	{
		target->width = info->get_texinfo().width;
		target->height = info->get_texinfo().height;
	}
	else
	{
		target->width = d3d->get_width();
		target->height = d3d->get_height();
	}

	HRESULT result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, target->target[0]);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);
	result = (*d3dintf->device.clear)(d3d->get_device(), 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);
	result = (*d3dintf->device.set_render_target)(d3d->get_device(), 0, backbuffer);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);

	target->screen_index = screen_index;
	target->page_index = page_index;

	cache_target* cache = find_cache_target(target->screen_index, target->width, target->height);
	if (cache == NULL)
	{
		if (!add_cache_target(d3d, info, width, height, xprescale, yprescale, target->screen_index))
		{
			global_free(target);
			return false;
		}
	}

	target->next = targethead;
	target->prev = NULL;

	if (targethead != NULL)
	{
		targethead->prev = target;
	}
	targethead = target;

	return true;
}

//============================================================
//  shaders::enumerate_screens
//============================================================
void shaders::enumerate_screens()
{
	screen_device_iterator iter(window->machine().root_device());
	num_screens = iter.count();
}


//============================================================
//  shaders::register_texture(texture::info)
//============================================================

bool shaders::register_texture(texture_info *texture)
{
	int width = texture->get_width();
	int height = texture->get_height();
	int xscale = texture->get_xscale();
	int yscale = texture->get_yscale();

	if (!master_enable || !d3dintf->post_fx_available)
	{
		return false;
	}

	enumerate_screens();

	renderer *d3d = (renderer *)window->drawdata;

	int hlsl_prescale_x = prescale_force_x;
	int hlsl_prescale_y = prescale_force_y;

	// Find the nearest prescale factor that is over our screen size
	if (hlsl_prescale_x == 0)
	{
		hlsl_prescale_x = 1;
		while (width * xscale * hlsl_prescale_x <= d3d->get_width())
		{
			hlsl_prescale_x++;
		}
		hlsl_prescale_x--;
	}

	if (hlsl_prescale_y == 0)
	{
		hlsl_prescale_y = 1;
		while (height * yscale * hlsl_prescale_y <= d3d->get_height())
		{
			hlsl_prescale_y++;
		}
		hlsl_prescale_y--;
	}

	hlsl_prescale_x = ((hlsl_prescale_x == 0) ? 1 : hlsl_prescale_x);
	hlsl_prescale_y = ((hlsl_prescale_y == 0) ? 1 : hlsl_prescale_y);

	if (!add_render_target(d3d, texture, width, height, xscale * hlsl_prescale_x, yscale * hlsl_prescale_y))
		return false;

	options->params_dirty = true;

	return true;
}

//============================================================
//  shaders::delete_resources
//============================================================

void shaders::delete_resources(bool reset)
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	initialized = false;

	cache_target *currcache = cachehead;
	while(cachehead != NULL)
	{
		cachehead = currcache->next;
		global_free(currcache);
		currcache = cachehead;
	}

	render_target *currtarget = targethead;
	while(targethead != NULL)
	{
		targethead = currtarget->next;
		global_free(currtarget);
		currtarget = targethead;
	}

	if (downsample_effect != NULL)
	{
		delete downsample_effect;
		downsample_effect = NULL;
	}
	if (bloom_effect != NULL)
	{
		delete bloom_effect;
		bloom_effect = NULL;
	}
	if (vector_effect != NULL)
	{
		delete vector_effect;
		vector_effect = NULL;
	}
	if (default_effect != NULL)
	{
		delete default_effect;
		default_effect = NULL;
	}
	if (post_effect != NULL)
	{
		delete post_effect;
		post_effect = NULL;
	}
	if (prescale_effect != NULL)
	{
		delete prescale_effect;
		prescale_effect = NULL;
	}
	if (pincushion_effect != NULL)
	{
		delete pincushion_effect;
		pincushion_effect = NULL;
	}
	if (phosphor_effect != NULL)
	{
		delete phosphor_effect;
		phosphor_effect = NULL;
	}
	if (focus_effect != NULL)
	{
		delete focus_effect;
		focus_effect = NULL;
	}
	if (deconverge_effect != NULL)
	{
		delete deconverge_effect;
		deconverge_effect = NULL;
	}
	if (color_effect != NULL)
	{
		delete color_effect;
		color_effect = NULL;
	}
	if (yiq_encode_effect != NULL)
	{
		delete yiq_encode_effect;
		yiq_encode_effect = NULL;
	}
	if (yiq_decode_effect != NULL)
	{
		delete yiq_decode_effect;
		yiq_decode_effect = NULL;
	}

	if (backbuffer != NULL)
	{
		(*d3dintf->surface.release)(backbuffer);
		backbuffer = NULL;
	}

	if (black_surface != NULL)
	{
		(*d3dintf->surface.release)(black_surface);
		black_surface = NULL;
	}
	if (black_texture != NULL)
	{
		(*d3dintf->texture.release)(black_texture);
		black_texture = NULL;
	}

	if (avi_copy_texture != NULL)
	{
		(*d3dintf->texture.release)(avi_copy_texture);
		avi_copy_texture = NULL;
	}

	if (avi_copy_surface != NULL)
	{
		(*d3dintf->surface.release)(avi_copy_surface);
		avi_copy_surface = NULL;
	}

	if (avi_final_texture != NULL)
	{
		(*d3dintf->texture.release)(avi_final_texture);
		avi_final_texture = NULL;
	}

	if (avi_final_target != NULL)
	{
		(*d3dintf->surface.release)(avi_final_target);
		avi_final_target = NULL;
	}

	shadow_bitmap.reset();
}


//============================================================
//  get_vector
//============================================================

static void get_vector(const char *data, int count, float *out, int report_error)
{
	if (count > 3)
	{
		if (sscanf(data, "%f,%f,%f,%f", &out[0], &out[1], &out[2], &out[3]) < 4 && report_error)
			mame_printf_error("Illegal quad vector value = %s\n", data);
	}
	else if(count > 2)
	{
		if (sscanf(data, "%f,%f,%f", &out[0], &out[1], &out[2]) < 3 && report_error)
			mame_printf_error("Illegal triple vector value = %s\n", data);
	}
	else if(count > 1)
	{
		if (sscanf(data, "%f,%f", &out[0], &out[1]) < 2 && report_error)
			mame_printf_error("Illegal double vector value = %s\n", data);
	}
	else if(count > 0)
	{
		if (sscanf(data, "%f", &out[0]) < 1 && report_error)
			mame_printf_error("Illegal single vector value = %s\n", data);
	}
}


/*-------------------------------------------------
    slider_alloc - allocate a new slider entry
    currently duplicated from ui.c, this could
    be done in a more ideal way.
-------------------------------------------------*/

static slider_state *slider_alloc(running_machine &machine, const char *title, INT32 minval, INT32 defval, INT32 maxval, INT32 incval, slider_update update, void *arg)
{
	int size = sizeof(slider_state) + strlen(title);
	slider_state *state = (slider_state *)auto_alloc_array_clear(machine, UINT8, size);

	state->minval = minval;
	state->defval = defval;
	state->maxval = maxval;
	state->incval = incval;
	state->update = update;
	state->arg = arg;
	strcpy(state->description, title);

	return state;
}


//============================================================
//  assorted global slider accessors
//============================================================

static INT32 slider_set(float *option, float scale, const char *fmt, astring *string, INT32 newval)
{
	if (option != NULL && newval != SLIDER_NOCHANGE) *option = (float)newval * scale;
	if (string != NULL) string->printf(fmt, *option);
	return floor(*option / scale + 0.5f);
}

static INT32 slider_shadow_mask_alpha(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->shadow_mask_alpha), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_shadow_mask_x_count(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	hlsl_options *options = (hlsl_options*)arg;
	if (newval != SLIDER_NOCHANGE) options->shadow_mask_count_x = newval;
	if (string != NULL) string->printf("%d", options->shadow_mask_count_x);
	options->params_dirty = true;
	return options->shadow_mask_count_x;
}

static INT32 slider_shadow_mask_y_count(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	hlsl_options *options = (hlsl_options*)arg;
	if (newval != SLIDER_NOCHANGE) options->shadow_mask_count_y = newval;
	if (string != NULL) string->printf("%d", options->shadow_mask_count_y);
	options->params_dirty = true;
	return options->shadow_mask_count_y;
}

static INT32 slider_shadow_mask_usize(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->shadow_mask_u_size), 1.0f / 32.0f, "%2.5f", string, newval);
}

static INT32 slider_shadow_mask_vsize(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->shadow_mask_v_size), 1.0f / 32.0f, "%2.5f", string, newval);
}

static INT32 slider_curvature(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->curvature), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_pincushion(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->pincushion), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_scanline_alpha(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->scanline_alpha), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_scanline_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->scanline_scale), 0.05f, "%2.2f", string, newval);
}

static INT32 slider_scanline_height(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->scanline_height), 0.05f, "%2.2f", string, newval);
}

static INT32 slider_scanline_bright_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->scanline_bright_scale), 0.05f, "%2.2f", string, newval);
}

static INT32 slider_scanline_bright_offset(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->scanline_bright_offset), 0.05f, "%2.2f", string, newval);
}

static INT32 slider_scanline_offset(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->scanline_offset), 0.05f, "%2.2f", string, newval);
}

static INT32 slider_defocus_x(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->defocus[0]), 0.5f, "%2.1f", string, newval);
}

static INT32 slider_defocus_y(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->defocus[1]), 0.5f, "%2.1f", string, newval);
}

static INT32 slider_red_converge_x(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->converge_x[0]), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_red_converge_y(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->converge_y[0]), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_green_converge_x(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->converge_x[1]), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_green_converge_y(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->converge_y[1]), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_blue_converge_x(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->converge_x[2]), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_blue_converge_y(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->converge_y[2]), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_red_radial_converge_x(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->radial_converge_x[0]), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_red_radial_converge_y(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->radial_converge_y[0]), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_green_radial_converge_x(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->radial_converge_x[1]), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_green_radial_converge_y(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->radial_converge_y[1]), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_blue_radial_converge_x(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->radial_converge_x[2]), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_blue_radial_converge_y(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->radial_converge_y[2]), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_red_from_r(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->red_ratio[0]), 0.005f, "%2.3f", string, newval);
}

static INT32 slider_red_from_g(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->red_ratio[1]), 0.005f, "%2.3f", string, newval);
}

static INT32 slider_red_from_b(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->red_ratio[2]), 0.005f, "%2.3f", string, newval);
}

static INT32 slider_green_from_r(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->grn_ratio[0]), 0.005f, "%2.3f", string, newval);
}

static INT32 slider_green_from_g(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->grn_ratio[1]), 0.005f, "%2.3f", string, newval);
}

static INT32 slider_green_from_b(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->grn_ratio[2]), 0.005f, "%2.3f", string, newval);
}

static INT32 slider_blue_from_r(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->blu_ratio[0]), 0.005f, "%2.3f", string, newval);
}

static INT32 slider_blue_from_g(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->blu_ratio[1]), 0.005f, "%2.3f", string, newval);
}

static INT32 slider_blue_from_b(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->blu_ratio[2]), 0.005f, "%2.3f", string, newval);
}

static INT32 slider_red_offset(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->offset[0]), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_green_offset(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->offset[1]), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_blue_offset(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->offset[2]), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_red_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->scale[0]), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_green_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->scale[1]), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_blue_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->scale[2]), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_red_power(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->power[0]), 0.05f, "%2.2f", string, newval);
}

static INT32 slider_green_power(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->power[1]), 0.05f, "%2.2f", string, newval);
}

static INT32 slider_blue_power(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->power[2]), 0.05f, "%2.2f", string, newval);
}

static INT32 slider_red_floor(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->floor[0]), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_green_floor(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->floor[1]), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_blue_floor(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->floor[2]), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_red_phosphor_life(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->phosphor[0]), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_green_phosphor_life(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->phosphor[1]), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_blue_phosphor_life(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->phosphor[2]), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_saturation(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->saturation), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_vector_attenuation(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->vector_length_scale), 0.01f, "%1.2f", string, newval);
}

static INT32 slider_vector_length_max(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->vector_length_ratio), 1.0f, "%4f", string, newval);
}

static INT32 slider_vector_bloom_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->vector_bloom_scale), 0.001f, "%1.3f", string, newval);
}

static INT32 slider_raster_bloom_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->raster_bloom_scale), 0.001f, "%1.3f", string, newval);
}

static INT32 slider_bloom_lvl0_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->bloom_level0_weight), 0.01f, "%1.2f", string, newval);
}

static INT32 slider_bloom_lvl1_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->bloom_level1_weight), 0.01f, "%1.2f", string, newval);
}

static INT32 slider_bloom_lvl2_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->bloom_level2_weight), 0.01f, "%1.2f", string, newval);
}

static INT32 slider_bloom_lvl3_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->bloom_level3_weight), 0.01f, "%1.2f", string, newval);
}

static INT32 slider_bloom_lvl4_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->bloom_level4_weight), 0.01f, "%1.2f", string, newval);
}

static INT32 slider_bloom_lvl5_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->bloom_level5_weight), 0.01f, "%1.2f", string, newval);
}

static INT32 slider_bloom_lvl6_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->bloom_level6_weight), 0.01f, "%1.2f", string, newval);
}

static INT32 slider_bloom_lvl7_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->bloom_level7_weight), 0.01f, "%1.2f", string, newval);
}

static INT32 slider_bloom_lvl8_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->bloom_level8_weight), 0.01f, "%1.2f", string, newval);
}

static INT32 slider_bloom_lvl9_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->bloom_level9_weight), 0.01f, "%1.2f", string, newval);
}

static INT32 slider_bloom_lvl10_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->bloom_level10_weight), 0.01f, "%1.2f", string, newval);
}

//============================================================
//  init_slider_list
//============================================================

shaders::slider_desc shaders::s_sliders[] =
{
	{ "Shadow Mask Darkness",                0,     0,   100, 1, slider_shadow_mask_alpha },
	{ "Shadow Mask X Count",                 1,   320,  1024, 1, slider_shadow_mask_x_count },
	{ "Shadow Mask Y Count",                 1,   240,  1024, 1, slider_shadow_mask_y_count },
	{ "Shadow Mask Pixel Count X",           1,     6,    64, 1, slider_shadow_mask_usize },
	{ "Shadow Mask Pixel Count Y",           1,     7,    64, 1, slider_shadow_mask_vsize },
	{ "Shadow Mask Pixel Count Y",           1,     7,    64, 1, slider_shadow_mask_vsize },
	{ "Shadow Mask Pixel Count Y",           1,     7,    64, 1, slider_shadow_mask_vsize },
	{ "Shadow Mask Pixel Count Y",           1,     7,    64, 1, slider_shadow_mask_vsize },
	{ "Screen Curvature",                    0,     3,   100, 1, slider_curvature },
	{ "Image Pincushion",                    0,     3,   100, 1, slider_pincushion },
	{ "Scanline Darkness",                   0,   100,   100, 1, slider_scanline_alpha },
	{ "Scanline Screen Height",              1,    20,    80, 1, slider_scanline_scale },
	{ "Scanline Indiv. Height",              1,    20,    80, 1, slider_scanline_height },
	{ "Scanline Brightness",                 0,    20,    40, 1, slider_scanline_bright_scale },
	{ "Scanline Brightness Overdrive",       0,     0,    20, 1, slider_scanline_bright_offset },
	{ "Scanline Jitter",                     0,     0,    40, 1, slider_scanline_offset },
	{ "Defocus X",                           0,     0,    64, 1, slider_defocus_x },
	{ "Defocus Y",                           0,     0,    64, 1, slider_defocus_y },
	{ "Red Position Offset X",           -1500,     3,  1500, 1, slider_red_converge_x },
	{ "Red Position Offset Y",           -1500,     0,  1500, 1, slider_red_converge_y },
	{ "Green Position Offset X",         -1500,     0,  1500, 1, slider_green_converge_x },
	{ "Green Position Offset Y",         -1500,     3,  1500, 1, slider_green_converge_y },
	{ "Blue Position Offset X",          -1500,     3,  1500, 1, slider_blue_converge_x },
	{ "Blue Position Offset Y",          -1500,     3,  1500, 1, slider_blue_converge_y },
	{ "Red Convergence X",               -1500,     0,  1500, 1, slider_red_radial_converge_x },
	{ "Red Convergence Y",               -1500,     0,  1500, 1, slider_red_radial_converge_y },
	{ "Green Convergence X",             -1500,     0,  1500, 1, slider_green_radial_converge_x },
	{ "Green Convergence Y",             -1500,     0,  1500, 1, slider_green_radial_converge_y },
	{ "Blue Convergence X",              -1500,     0,  1500, 1, slider_blue_radial_converge_x },
	{ "Blue Convergence Y",              -1500,     0,  1500, 1, slider_blue_radial_converge_y },
	{ "Red Output from Red Input",        -400,     0,   400, 5, slider_red_from_r },
	{ "Red Output from Green Input",      -400,     0,   400, 5, slider_red_from_g },
	{ "Red Output from Blue Input",       -400,     0,   400, 5, slider_red_from_b },
	{ "Green Output from Red Input",      -400,     0,   400, 5, slider_green_from_r },
	{ "Green Output from Green Input",    -400,     0,   400, 5, slider_green_from_g },
	{ "Green Output from Blue Input",     -400,     0,   400, 5, slider_green_from_b },
	{ "Blue Output from Red Input",       -400,     0,   400, 5, slider_blue_from_r },
	{ "Blue Output from Green Input",     -400,     0,   400, 5, slider_blue_from_g },
	{ "Blue Output from Blue Input",      -400,     0,   400, 5, slider_blue_from_b },
	{ "Saturation",                          0,   140,   400, 1, slider_saturation },
	{ "Red DC Offset",                    -100,     0,   100, 1, slider_red_offset },
	{ "Green DC Offset",                  -100,     0,   100, 1, slider_green_offset },
	{ "Blue DC Offset",                   -100,     0,   100, 1, slider_blue_offset },
	{ "Red Scale",                        -200,    95,   200, 1, slider_red_scale },
	{ "Green Scale",                      -200,    95,   200, 1, slider_green_scale },
	{ "Blue Scale",                       -200,    95,   200, 1, slider_blue_scale },
	{ "Red Gamma",                         -80,    16,    80, 1, slider_red_power },
	{ "Green Gamma",                       -80,    16,    80, 1, slider_green_power },
	{ "Blue Gamma",                        -80,    16,    80, 1, slider_blue_power },
	{ "Red Floor",                           0,     5,   100, 1, slider_red_floor },
	{ "Green Floor",                         0,     5,   100, 1, slider_green_floor },
	{ "Blue Floor",                          0,     5,   100, 1, slider_blue_floor },
	{ "Red Phosphor Life",                   0,    40,   100, 1, slider_red_phosphor_life },
	{ "Green Phosphor Life",                 0,    40,   100, 1, slider_green_phosphor_life },
	{ "Blue Phosphor Life",                  0,    40,   100, 1, slider_blue_phosphor_life },
	{ "Vector Length Attenuation",           0,    80,   100, 1, slider_vector_attenuation },
	{ "Vector Attenuation Length Limit",     1,   500,  1000, 1, slider_vector_length_max },
	{ "Vector Bloom Scale",                  0,   300,  1000, 5, slider_vector_bloom_scale },
	{ "Raster Bloom Scale",                  0,   225,  1000, 5, slider_raster_bloom_scale },
	{ "Bloom Level 0 Scale",                 0,   100,   100, 1, slider_bloom_lvl0_scale },
	{ "Bloom Level 1 Scale",                 0,    21,   100, 1, slider_bloom_lvl1_scale },
	{ "Bloom Level 2 Scale",                 0,    19,   100, 1, slider_bloom_lvl2_scale },
	{ "Bloom Level 3 Scale",                 0,    17,   100, 1, slider_bloom_lvl3_scale },
	{ "Bloom Level 4 Scale",                 0,    15,   100, 1, slider_bloom_lvl4_scale },
	{ "Bloom Level 5 Scale",                 0,    14,   100, 1, slider_bloom_lvl5_scale },
	{ "Bloom Level 6 Scale",                 0,    13,   100, 1, slider_bloom_lvl6_scale },
	{ "Bloom Level 7 Scale",                 0,    12,   100, 1, slider_bloom_lvl7_scale },
	{ "Bloom Level 8 Scale",                 0,    11,   100, 1, slider_bloom_lvl8_scale },
	{ "Bloom Level 9 Scale",                 0,    10,   100, 1, slider_bloom_lvl9_scale },
	{ "Bloom Level 10 Scale",                0,     9,   100, 1, slider_bloom_lvl10_scale },
	{ NULL, 0, 0, 0, 0, NULL },
};

slider_state *shaders::init_slider_list()
{
	if (!master_enable || !d3dintf->post_fx_available)
	{
		g_slider_list = NULL;
		return NULL;
	}

	slider_state *listhead = NULL;
	slider_state **tailptr = &listhead;

	for (int index = 0; s_sliders[index].name != NULL; index++)
	{
		slider_desc *slider = &s_sliders[index];
		*tailptr = slider_alloc(window->machine(), slider->name, slider->minval, slider->defval, slider->maxval, slider->step, slider->adjustor, (void*)options);
		tailptr = &(*tailptr)->next;
	}

	return listhead;
}

};

//============================================================
//  get_slider_list
//============================================================

void *windows_osd_interface::get_slider_list()
{
	return (void*)g_slider_list;
}



// NOTE: The function below is taken directly from src/emu/video.c and should likely be moved into a global helper function.
//-------------------------------------------------
//  open_next - open the next non-existing file of
//  type filetype according to our numbering
//  scheme
//-------------------------------------------------

static file_error open_next(d3d::renderer *d3d, emu_file &file, const char *templ, const char *extension, int idx)
{
	UINT32 origflags = file.openflags();

	// handle defaults
	const char *snapname = templ ? templ : d3d->get_window()->machine().options().snap_name();

	if (snapname == NULL || snapname[0] == 0)
		snapname = "%g/%i";
	astring snapstr(snapname);

	// strip any extension in the provided name
	int index = snapstr.rchr(0, '.');
	if (index != -1)
		snapstr.substr(0, index);

	// handle %d in the template (for image devices)
	astring snapdev("%d_");
	int pos = snapstr.find(0, snapdev);

	if (pos != -1)
	{
		// if more %d are found, revert to default and ignore them all
		if (snapstr.find(pos + 3, snapdev) != -1)
			snapstr.cpy("%g/%i");
		// else if there is a single %d, try to create the correct snapname
		else
		{
			int name_found = 0;

			// find length of the device name
			int end1 = snapstr.find(pos + 3, "/");
			int end2 = snapstr.find(pos + 3, "%");
			int end = -1;

			if ((end1 != -1) && (end2 != -1))
				end = MIN(end1, end2);
			else if (end1 != -1)
				end = end1;
			else if (end2 != -1)
				end = end2;
			else
				end = snapstr.len();

			if (end - pos < 3)
				fatalerror("Something very wrong is going on!!!\n");

			// copy the device name to an astring
			astring snapdevname;
			snapdevname.cpysubstr(snapstr, pos + 3, end - pos - 3);

			// verify that there is such a device for this system
			image_interface_iterator iter(d3d->get_window()->machine().root_device());
			for (device_image_interface *image = iter.first(); image != NULL; iter.next())
			{
				// get the device name
				astring tempdevname(image->brief_instance_name());

				if (snapdevname.cmp(tempdevname) == 0)
				{
					// verify that such a device has an image mounted
					if (image->basename() != NULL)
					{
						astring filename(image->basename());

						// strip extension
						filename.substr(0, filename.rchr(0, '.'));

						// setup snapname and remove the %d_
						snapstr.replace(0, snapdevname, filename);
						snapstr.del(pos, 3);

						name_found = 1;
					}
				}
			}

			// or fallback to default
			if (name_found == 0)
				snapstr.cpy("%g/%i");
		}
	}

	// add our own index
	// add our own extension
	snapstr.cat(".").cat(extension);

	// substitute path and gamename up front
	snapstr.replace(0, "/", PATH_SEPARATOR);
	snapstr.replace(0, "%g", d3d->get_window()->machine().basename());

	// determine if the template has an index; if not, we always use the same name
	astring fname;
	if (snapstr.find(0, "%i") == -1)
		fname.cpy(snapstr);

	// otherwise, we scan for the next available filename
	else
	{
		// try until we succeed
		astring seqtext;
		file.set_openflags(OPEN_FLAG_READ);
		for (int seq = 0; ; seq++)
		{
			// build up the filename
			fname.cpy(snapstr).replace(0, "%i", seqtext.format("%04d_%d", seq, idx).cstr());

			// try to open the file; stop when we fail
			file_error filerr = file.open(fname);
			if (filerr != FILERR_NONE)
				break;
		}
	}

	// create the final file
	file.set_openflags(origflags);
	return file.open(fname);
}
