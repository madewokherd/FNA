/*
 * Copyright 2020 Rémi Bernon for CodeWeavers
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <windows.h>
#include <stdio.h>

#define COBJMACROS
#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>

#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(fnamf);

struct context
{
    IMFSourceReader *reader;
    DWORD video_stream;
    DWORD audio_stream;

    IMFMediaBuffer *audio_buffer;
    BOOL eos;
};

static void set_audio_media_type(struct context *ctx)
{
    IMFMediaType *type;
    HRESULT hr;

    IMFSourceReader_SetStreamSelection(ctx->reader, ctx->audio_stream, FALSE);

    if (SUCCEEDED(hr = MFCreateMediaType(&type)))
    {
        hr = IMFMediaType_SetGUID(type, &MF_MT_MAJOR_TYPE, &MFMediaType_Audio);

        if (SUCCEEDED(hr))
            hr = IMFMediaType_SetGUID(type, &MF_MT_SUBTYPE, &MFAudioFormat_Float);

        if (SUCCEEDED(hr))
            hr = IMFSourceReader_SetCurrentMediaType(ctx->reader, ctx->audio_stream, NULL, type);

        IMFMediaType_Release(type);
    }

    IMFSourceReader_SetStreamSelection(ctx->reader, ctx->audio_stream, SUCCEEDED(hr));
}

static void set_video_media_type(struct context *ctx)
{
    IMFMediaType *type;
    HRESULT hr;

    IMFSourceReader_SetStreamSelection(ctx->reader, ctx->video_stream, FALSE);

    if (SUCCEEDED(hr = MFCreateMediaType(&type)))
    {
        hr = IMFMediaType_SetGUID(type, &MF_MT_MAJOR_TYPE, &MFMediaType_Video);

        if (SUCCEEDED(hr))
            hr = IMFMediaType_SetGUID(type, &MF_MT_SUBTYPE, &MFVideoFormat_I420);

        if (SUCCEEDED(hr))
            hr = IMFSourceReader_SetCurrentMediaType(ctx->reader, ctx->video_stream, NULL, type);

        IMFMediaType_Release(type);
    }

    IMFSourceReader_SetStreamSelection(ctx->reader, ctx->video_stream, SUCCEEDED(hr));
}

void CDECL tf_fopen(const WCHAR *url, struct context **ctx)
{
    IMFSourceReader *reader;
    IMFAttributes *attributes;
    HRESULT hr;

    WINE_TRACE("url %s, ctx %p.\n", wine_dbgstr_w(url), ctx);

    *ctx = NULL;
    if (FAILED(hr = MFCreateAttributes(&attributes, 1)))
        return;

    hr = IMFAttributes_SetUINT32(attributes, &MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);
    if (SUCCEEDED(hr))
    {
        hr = MFCreateSourceReaderFromURL(url, attributes, &reader);
    }
    IMFAttributes_Release(attributes);

    if (SUCCEEDED(hr))
    {
        if (!(*ctx = calloc(1, sizeof(**ctx))))
            IMFSourceReader_Release(reader);
        else
        {
            (*ctx)->reader = reader;
            (*ctx)->video_stream = MF_SOURCE_READER_FIRST_VIDEO_STREAM;
            (*ctx)->audio_stream = MF_SOURCE_READER_FIRST_AUDIO_STREAM;
            set_video_media_type(*ctx);
            set_audio_media_type(*ctx);
        }
    }

    WINE_TRACE("ret ctx %p, reader %p, hr %#x.\n", *ctx, reader, hr);
}

void CDECL tf_close(struct context *ctx)
{
    WINE_TRACE("ctx %p.\n", ctx);
    if (ctx->audio_buffer) IMFMediaBuffer_Release(ctx->audio_buffer);
    IMFSourceReader_Release(ctx->reader);
    free(ctx);
}

INT CDECL tf_hasaudio(struct context *ctx)
{
    IMFMediaType *type;
    HRESULT hr;
    BOOL selected;
    GUID guid;

    WINE_TRACE("ctx %p.\n", ctx);

    hr = IMFSourceReader_GetStreamSelection(ctx->reader, ctx->audio_stream, &selected);
    if (FAILED(hr) || !selected)
        return FALSE;

    hr = IMFSourceReader_GetCurrentMediaType(ctx->reader, ctx->audio_stream, &type);
    if (FAILED(hr))
        return FALSE;

    hr = IMFMediaType_GetGUID(type, &MF_MT_MAJOR_TYPE, &guid);
    IMFMediaType_Release(type);
    return SUCCEEDED(hr) && IsEqualGUID(&guid, &MFMediaType_Audio);
}

INT CDECL tf_hasvideo(struct context *ctx)
{
    IMFMediaType *type;
    HRESULT hr;
    BOOL selected;
    GUID guid;

    WINE_TRACE("ctx %p.\n", ctx);

    hr = IMFSourceReader_GetStreamSelection(ctx->reader, ctx->video_stream, &selected);
    if (FAILED(hr) || !selected)
        return FALSE;

    hr = IMFSourceReader_GetCurrentMediaType(ctx->reader, ctx->video_stream, &type);
    if (FAILED(hr))
        return FALSE;

    hr = IMFMediaType_GetGUID(type, &MF_MT_MAJOR_TYPE, &guid);
    IMFMediaType_Release(type);
    return SUCCEEDED(hr) && IsEqualGUID(&guid, &MFMediaType_Video);
}

void CDECL tf_videoinfo(struct context *ctx, INT *width, INT *height, double *fps, INT *format)
{
    IMFMediaType *type;
    HRESULT hr;
    UINT64 size, rate;
    BOOL selected;

    *width = *height = *fps = *format = 0;

    WINE_TRACE("ctx %p, width %p, height %p, format %p, format %p.\n", ctx, width, height, fps, format);

    hr = IMFSourceReader_GetStreamSelection(ctx->reader, ctx->video_stream, &selected);
    if (FAILED(hr) || !selected)
        return;

    hr = IMFSourceReader_GetCurrentMediaType(ctx->reader, ctx->video_stream, &type);
    if (FAILED(hr))
        return;

    hr = IMFMediaType_GetUINT64(type, &MF_MT_FRAME_SIZE, &size);
    if (SUCCEEDED(hr))
        hr = IMFMediaType_GetUINT64(type, &MF_MT_FRAME_RATE, &rate);

    if (SUCCEEDED(hr))
    {
        *format = 0 /* I420 */;
        *width = (UINT32)(size >> 32);
        *height = (UINT32)(size >> 0);
        *fps = (double)(rate >> 32) / (double)(rate & 0xffffffff);
    }

    IMFMediaType_Release(type);
    WINE_TRACE("ret width %d, height %d, fps %f, format %d, hr %#x.\n", *width, *height, *fps, *format, hr);
}

void CDECL tf_audioinfo(struct context *ctx, INT *channels, INT *samplerate)
{
    IMFMediaType *type;
    HRESULT hr;
    BOOL selected;

    *channels = 0;
    *samplerate = 0;

    WINE_TRACE("ctx %p, channels %p, samplerate %p.\n", ctx, channels, samplerate);

    hr = IMFSourceReader_GetStreamSelection(ctx->reader, ctx->audio_stream, &selected);
    if (FAILED(hr) || !selected)
        return;

    hr = IMFSourceReader_GetCurrentMediaType(ctx->reader, ctx->audio_stream, &type);
    if (FAILED(hr))
        return;

    hr = IMFMediaType_GetUINT32(type, &MF_MT_AUDIO_NUM_CHANNELS, (UINT32 *)channels);
    if (SUCCEEDED(hr))
        hr = IMFMediaType_GetUINT32(type, &MF_MT_AUDIO_SAMPLES_PER_SECOND, (UINT32 *)samplerate);

    if (FAILED(hr))
    {
        *channels = 0;
        *samplerate = 0;
    }

    IMFMediaType_Release(type);
    WINE_TRACE("ret channels %d, samplerate %d, hr %#x.\n", *channels, *samplerate, hr);
}

INT CDECL tf_setaudiotrack(struct context *ctx, INT track)
{
    IMFMediaType *type;
    HRESULT hr;
    DWORD i = 0;
    GUID guid;

    WINE_TRACE("ctx %p.\n", ctx);

    while (1)
    {
        hr = IMFSourceReader_GetCurrentMediaType(ctx->reader, i++, &type);
        if (FAILED(hr))
            return FALSE;

        hr = IMFMediaType_GetGUID(type, &MF_MT_MAJOR_TYPE, &guid);
        IMFMediaType_Release(type);
        if (FAILED(hr))
            return FALSE;

        if (IsEqualGUID(&guid, &MFMediaType_Audio) && !track--)
            break;
    }

    ctx->audio_stream = i - 1;
    set_audio_media_type(ctx);
    return TRUE;
}

INT CDECL tf_eos(struct context *ctx, INT track)
{
    WINE_TRACE("ctx %p.\n", ctx);
    return ctx->eos;
}

void CDECL tf_reset(struct context *ctx)
{
    PROPVARIANT pos;
    HRESULT hr;

    WINE_TRACE("ctx %p.\n", ctx);

    pos.vt = VT_I8;
    pos.hVal.QuadPart = 0;
    hr = IMFSourceReader_SetCurrentPosition(ctx->reader, &GUID_NULL, &pos);
    if (FAILED(hr))
        WINE_ERR("IMFSourceReader_SetCurrentPosition returned %#x.\n", hr);
}

INT CDECL tf_readvideo(struct context *ctx, void *dst, int numframes)
{
    IMFMediaBuffer *buffer;
    IMFSample *sample;
    HRESULT hr;
    DWORD flags, len = 0;
    BYTE *src;

    WINE_TRACE("ctx %p, dst %p, numframes %d.\n", ctx, dst, numframes);
    while (1)
    {
        hr = IMFSourceReader_ReadSample(ctx->reader, ctx->video_stream, 0, NULL, &flags, NULL, &sample);
        if (flags & MF_SOURCE_READERF_ENDOFSTREAM) ctx->eos = TRUE;

        if (FAILED(hr) || !sample)
            return FALSE;

        if (!numframes--)
            break;
        IMFSample_Release(sample);
    }

    hr = IMFSample_ConvertToContiguousBuffer(sample, &buffer);
    IMFSample_Release(sample);
    if (FAILED(hr))
        return FALSE;

    hr = IMFMediaBuffer_Lock(buffer, &src, NULL, &len);
    if (SUCCEEDED(hr))
    {
        memcpy(dst, src, len);
        IMFMediaBuffer_Unlock(buffer);
    }
    IMFMediaBuffer_Release(buffer);

    WINE_TRACE("ret len %d, hr %#x\n", len, hr);

    return len ? TRUE : FALSE;
}

INT CDECL tf_readaudio(struct context *ctx, float *dst, int dst_count)
{
    IMFSample *sample;
    HRESULT hr;
    DWORD len = 0, flags;
    float *src;

    WINE_TRACE("ctx %p, dst %p, dst_count %d.\n", ctx, dst, dst_count);

    if (ctx->audio_buffer)
        hr = S_OK;
    else
    {
        hr = IMFSourceReader_ReadSample(ctx->reader, ctx->audio_stream, 0, NULL, &flags, NULL, &sample);
        if (flags & MF_SOURCE_READERF_ENDOFSTREAM) ctx->eos = TRUE;

        if (SUCCEEDED(hr) && sample)
        {
            hr = IMFSample_ConvertToContiguousBuffer(sample, &ctx->audio_buffer);
            IMFSample_Release(sample);
        }
    }

    if (FAILED(hr))
        dst_count = -1;
    else if (!ctx->audio_buffer || ctx->eos)
        dst_count = 0;
    else
    {
        hr = IMFMediaBuffer_Lock(ctx->audio_buffer, (BYTE **)&src, NULL, &len);
        if (SUCCEEDED(hr))
        {
            dst_count = min(len / sizeof(float), dst_count);
            memcpy(dst, src, dst_count * sizeof(float));
            len -= dst_count * sizeof(float);

            memcpy(src, src + dst_count, len);
            hr = IMFMediaBuffer_SetCurrentLength(ctx->audio_buffer, len);
            IMFMediaBuffer_Unlock(ctx->audio_buffer);
        }

        if (FAILED(hr) || len == 0)
        {
            IMFMediaBuffer_Release(ctx->audio_buffer);
            ctx->audio_buffer = NULL;
        }
    }

    WINE_TRACE("ret len %d\n", dst_count);
    return dst_count;
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, void *res)
{
    WINE_TRACE("instance %p, reason %x, res %p\n", instance, reason, res);

    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        MFStartup(MF_VERSION, MFSTARTUP_FULL);
        break;
    case DLL_PROCESS_DETACH:
        MFShutdown();
        break;
    }
    return TRUE;
}
