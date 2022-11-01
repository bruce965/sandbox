Sandbox
=======

Run untrusted processes in a sandbox, blocking access to the system.

**This software is still very incomplete and probably not useful yet.**


## Working Principles

A _supervisor_ process is spawned.

A temporary [app _container_](https://docs.microsoft.com/en/windows/win32/secauthz/appcontainer-isolation)
with no privileges is created. Processes running in the _container_ have no access
to system resources such as device resources (camera, microphone, GPS, etc.), files,
the system registry, networking, or other processes running in the system.

The _untrusted process_ is spawned inside this _container_. Normally it would not
be able to access any resource. To solve this problem, before its code has a chance
to be executed, a set of _hooks_ is injected and all calls to the operating system's
kernel are replaced with calls to the _supervisor_.

At this point the _untrusted process_ is running inside the _container_, and can
not access any of the system resources. The only connection to the system is through
the _supervisor_. If this process tries to bypass the _hooks_, it will be blocked
by the operating system's kernel.

The _supervisor_ now has complete control over the resources requested from the
_untrusted process_. Each individual request can be monitored and granted or denied.


## Code Structure

### SandboxLauncher

The supervisor process _"SandboxLauncher.exe"_ manages the processes running in
the sandbox. It is written in C# .NET.

### Sandbox

"sandbox32.dll" and "sandbox64.dll" DLLs containing the C++ code used from the
supervisor.

### SandboxHooks

Hooks reside in the "sandbox-hooks32.dll" and "sandbox-hooks64.dll" DLLs, injected
through [Detours](https://github.com/microsoft/Detours).

These hooks intercept blocked calls to the operating system's kernel and redirect
them to the _supervisor_.

### SandboxShared

This is the shared code, namely utilities to maintain a connection between the
_supervisor_ and the _hooks_ injected in the _untrusted process_ running in the
_container_.

### Test

Test implementation of the _untrusted process_, used to esure that every operation
is properly intercepted and redirected to the _supervisor_.


## License

Check [the license](LICENSE) for details.
