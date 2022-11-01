using SandboxLauncher;
using System;

var guid = new Guid("{F21B9452-FB40-4754-B5C4-0E5CDBE69EFC}");

Console.WriteLine("Starting...");

using (var sandbox = new Sandbox("SandboxedLauncherTest", guid))
{
	//sandbox.StartProcess(@"""C:\Users\User\AppData\Local\Programs\Microsoft VS Code\Code.exe""");
	//sandbox.StartProcess(@"""notepad.exe""");
	sandbox.StartProcess(@"""Test.exe""");
	//sandbox.StartProcess(@"""C:\Users\User\Desktop\temp\Notepad++\notepad++.exe""");
	//sandbox.StartProcess(@"""C:\Program Files\HxD\HxD.exe""");
	//sandbox.StartProcess(@"""C:\Users\User\Desktop\temp\WinBuilds\bin\yypkg-1.5.0.exe""");
	//sandbox.StartProcess(@"""C:\Users\User\Desktop\temp\ProcExp\procexp64.exe""");
	//sandbox.StartProcess(@"""C:\Users\User\Desktop\temp\WinObj\Winobj.exe""");
	//sandbox.StartProcess(@"""X:\projects\Sandbox\SandboxLauncher\bin\Debug\net5.0\tiny.exe""");

	sandbox.PumpMessages();
}

Console.WriteLine("Finished.");
