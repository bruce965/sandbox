using System;
using System.Runtime.Serialization;

namespace SandboxLauncher
{

	[Serializable]
	public class SandboxException : Exception
	{
		public int ErrorCode { get; }

		public SandboxException()
			: base() { }

		public SandboxException(string? message, int errorCode)
			: base(message)
		{
			ErrorCode = errorCode;
		}

		public SandboxException(string? message, int errorCode, Exception? innerException)
			: base(message, innerException)
		{
			ErrorCode = errorCode;
		}

		protected SandboxException(SerializationInfo info, StreamingContext context)
			: base(info, context) { }
	}
}
