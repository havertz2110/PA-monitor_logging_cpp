# Monitor-logging-csharp

## Introduction
This is a project about making a Windows file system monitor (console application) that tracks changes within specified directory and its subdirectories and files, logging detailed information about each event in a structured format.

## Goals
### 1. Recursive Directory Monitoring:

Allow the user to specify one directory to monitor and utilize the ReadDirectoryChangesW Windows API function for efficient change detection.

### 2. Comprehensive Logging:

Create a log file (e.g., "file_monitor.log") to store event details.
Event Details: For each file system event, log the following information:
- Timestamp: Precise date and time the event occurred (e.g., "2024-06-13 12:22:35 AM").
- Event Type: Clearly indicate the type of change (Created, Modified, Deleted, Renamed (include both old and new file names)).
File/Directory Path: Full path of the affected file or directory (e.g., "C:\Documents\report.txt").
- Process Information (Optional): If possible, include the process ID and name responsible for the change (e.g., "PID: 1234, Process: notepad.exe").
Log Format:
- Use CSV log format (comma-separated values) to ensure easy parsing and analysis.
This is an example CSV log entry:

```2024-06-13 12:22:35 AM,Created,C:\Documents\report.txt,PID: 1234, Process: notepad.exe```

I also provide summary statistics on file changes over time to console output, including:
- Total number of changes
- Number of changes per event type

Error Handling:

- Gracefully handle errors that may occur during monitoring or logging.
- Log any errors encountered with relevant details.


## Docs I refer to

I use an API and this is a blog about how to use it ReadDirectoryChangesW API
https://developersarea.wordpress.com/2014/09/26/win32-file-watcher-api-to-monitor-directory-changes/

and microsoft documentation
https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-readdirectorychangesw

## What I still can not do

### The process id part
To display the process ID and name responsible for creating, deleting, or modifying files in the directory, you'll need to use some advanced techniques in Windows programming, such as the Windows Management Instrumentation (WMI) or Process Trace APIs. These APIs allow you to trace the file system changes back to the originating process.

.

An Alternative Approach:
If you want to keep it simpler, you could periodically scan the system's running processes and compare their file handles to the directory you're monitoring. This won't be as precise as the above methods but can give a reasonable approximation.

Using Process Trace APIs:
You can use Event Tracing for Windows (ETW) or the NT Kernel Logger to trace which process is making changes. However, this approach is quite complex and involves registering for file system change events and then correlating those events with process IDs

## Installation and file structure
