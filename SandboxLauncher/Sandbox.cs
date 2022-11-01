using System;
using System.Runtime.InteropServices;

namespace SandboxLauncher
{
    public class Sandbox : IDisposable
    {
		IntPtr handle;

		public Sandbox(string name, Guid guid)
		{
			if (!PInvoke.CreateSandbox(out handle, name, guid, out var errorMessage, out var errorCode))
				throw new SandboxException(errorMessage, errorCode);
		}

		/// <summary>
		/// Start a new process in the sandbox.
		/// </summary>
		/// <param name="commandLine">Command line to launch the process.</param>
		public void StartProcess(string commandLine)
		{
			CheckNotDisposed();

			if (!PInvoke.StartSandboxProcess(handle, commandLine, out var errorMessage, out var errorCode))
				throw new SandboxException(errorMessage, errorCode);
		}

		/// <summary>
		/// Start pumping messages from the sandbox.
		/// <para/>
		/// This method will block until all processes in the sandbox are terminated.
		/// </summary>
		public unsafe void PumpMessages()
		{
			while (PInvoke.GetSandboxMessage(handle, out var messageType, out var data))
			{
				Console.WriteLine("Received message: {0}", messageType);

				switch (messageType)
				{
					case MessageType.NtOpenFile:
						// TODO
						if (!PInvoke.ConfirmSandboxMessage(handle, IntPtr.Zero))
						{
							throw new Exception($"Error confirming sandbox message {messageType}: {Marshal.GetLastWin32Error()}");
						}
						return;

					case MessageType.NtCreateFile:
						// TODO
						if (!PInvoke.ConfirmSandboxMessage(handle, IntPtr.Zero))
						{
							throw new Exception($"Error confirming sandbox message {messageType}: {Marshal.GetLastWin32Error()}");
						}
						return;

					case MessageType.NtWriteFile:
						// TODO
						if (!PInvoke.ConfirmSandboxMessage(handle, IntPtr.Zero))
						{
							throw new Exception($"Error confirming sandbox message {messageType}: {Marshal.GetLastWin32Error()}");
						}
						return;

					default:
						throw new Exception($"Unknown message received from sandbox: {messageType}");
				}
			}
		}

		#region PInvoke

		enum MessageType : int
		{
			NtOpenFile = 0x00000001,
			NtCreateFile = 0x00000002,
			NtWriteFile = 0x00000003,
		}

		static class PInvoke
		{
#if TRUE
			const string SANDBOX_DLL = "sandbox32";
#else
			const string SANDBOX_DLL = "sandbox64";
#endif

			[DllImport(SANDBOX_DLL, CallingConvention = CallingConvention.Cdecl, SetLastError = true, CharSet = CharSet.Unicode)]
			public static extern bool CreateSandbox(out IntPtr sandbox, string name, Guid guid, [MarshalAs(UnmanagedType.BStr)] out string errorMessage, out int errorCode);

			[DllImport(SANDBOX_DLL, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
			public static extern bool StartSandboxProcess(IntPtr sandbox, string commandLine, [MarshalAs(UnmanagedType.BStr)] out string errorMessage, out int errorCode);

			[DllImport(SANDBOX_DLL, CallingConvention = CallingConvention.Cdecl)]
			public static extern bool GetSandboxMessage(IntPtr sandbox, out MessageType message, out IntPtr data);

			[DllImport(SANDBOX_DLL, CallingConvention = CallingConvention.Cdecl)]
			public static extern bool ConfirmSandboxMessage(IntPtr sandbox, IntPtr data);

			[DllImport(SANDBOX_DLL, CallingConvention = CallingConvention.Cdecl)]
			public static extern bool DestroySandbox(IntPtr sandbox);
		}

		#endregion

		#region IDisposable

		private bool disposedValue;

		void CheckNotDisposed()
		{
			if (disposedValue)
				throw new ObjectDisposedException(nameof(Sandbox));
		}

		protected virtual void Dispose(bool disposing)
		{
			if (!disposedValue)
			{
				if (disposing)
				{
					// dispose managed state (managed objects)
					// (nothing to do)
				}

				// free unmanaged resources (unmanaged objects) and override finalizer
				PInvoke.DestroySandbox(handle);

				// set large fields to null
				// (nothing to do)

				disposedValue = true;
			}
		}

		~Sandbox()
		{
			// Do not change this code. Put cleanup code in 'Dispose(bool disposing)' method
			Dispose(disposing: false);
		}

		public void Dispose()
		{
			// Do not change this code. Put cleanup code in 'Dispose(bool disposing)' method
			Dispose(disposing: true);
			GC.SuppressFinalize(this);
		}

		#endregion
	}
}
