using System;

namespace SandboxedLauncher
{
	class Program
	{
		static readonly Guid GUID = new Guid("{F21B9452-FB40-4754-B5C4-0E5CDBE69EFC}");

		static void Main(string[] args)
		{
			Console.WriteLine("Starting...");

			using (var sandbox = new Sandbox("SandboxedLauncherTest", GUID))
			{
				sandbox.StartProcess(@"""notepad.exe""");
				//sandbox.StartProcess(@"""..\..\..\..\Debug\Test.exe""");

				//sandbox.PumpMessages();
			}

			Console.WriteLine("Finished.");
		}
	}
}