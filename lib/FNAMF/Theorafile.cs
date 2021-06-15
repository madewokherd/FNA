/* 
 * Copyright (c) 2021 Rémi Bernon for CodeWeavers.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software in a
 * product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 *
 */

#region Using Statements
using System;
using System.Runtime.InteropServices;
using System.Text;
#endregion

public static class Theorafile
{
	#region Native Library Name

	const string nativeLibName = "FNAMF";

	#endregion

	#region libtheora Enumerations

	public enum th_pixel_fmt
	{
		TH_PF_420,
		TH_PF_RSVD,
		TH_PF_422,
		TH_PF_444,
		TH_PF_NFORMATS
	}

	#endregion

	#region Theorafile Implementation

	[DllImport(nativeLibName, EntryPoint = "tf_fopen", CallingConvention = CallingConvention.Cdecl)]
	private static extern void _tf_fopen([In, MarshalAs(UnmanagedType.LPWStr)] string url, [Out] out IntPtr reader);

	public static unsafe int tf_fopen(string fname, out IntPtr file)
	{
		_tf_fopen(fname, out file);
		if (file.Equals(0)) throw new NullReferenceException();
		return 0;
	}

	[DllImport(nativeLibName, EntryPoint = "tf_close", CallingConvention = CallingConvention.Cdecl)]
	private static extern void _tf_close([In] IntPtr file);

	public static int tf_close(ref IntPtr file)
	{
		_tf_close(file);
		file = IntPtr.Zero;
		return 0;
	}

	[DllImport(nativeLibName, CallingConvention = CallingConvention.Cdecl)]
	public static extern int tf_hasaudio(IntPtr file);

	[DllImport(nativeLibName, CallingConvention = CallingConvention.Cdecl)]
	public static extern int tf_hasvideo(IntPtr file);

	[DllImport(nativeLibName, CallingConvention = CallingConvention.Cdecl)]
	public static extern void tf_videoinfo(
		IntPtr file,
		out int width,
		out int height,
		out double fps,
		out th_pixel_fmt fmt
	);

	[DllImport(nativeLibName, CallingConvention = CallingConvention.Cdecl)]
	public static extern void tf_audioinfo(
		IntPtr file,
		out int channels,
		out int samplerate
	);

	[DllImport(nativeLibName, CallingConvention = CallingConvention.Cdecl)]
	public static extern int tf_setaudiotrack(IntPtr file, int track);

	[DllImport(nativeLibName, CallingConvention = CallingConvention.Cdecl)]
	public static extern int tf_eos(IntPtr file);

	[DllImport(nativeLibName, CallingConvention = CallingConvention.Cdecl)]
	public static extern void tf_reset(IntPtr file);

	[DllImport(nativeLibName, CallingConvention = CallingConvention.Cdecl)]
	public static extern int tf_readvideo(IntPtr file, IntPtr buffer, int numframes);

	[DllImport(nativeLibName, CallingConvention = CallingConvention.Cdecl)]
	public static extern int tf_readaudio(IntPtr file, IntPtr buffer, int length);

	#endregion
}
