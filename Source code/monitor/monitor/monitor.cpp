#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <assert.h>
#include <cstdint>
#include <stddef.h>
#include <filesystem>
#include <wchar.h>
#include <time.h>
#include <fstream>
#include <psapi.h> // Sử dụng EnumProcessModules và GetModuleFileNameExA
#pragma comment(lib, "Psapi.lib") // Liên kết với Psapi.lib

// Hàm lấy thời gian hệ thống hiện tại dưới dạng chuỗi có định dạng
void GetFormattedTime(char* buffer, size_t size) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    snprintf(buffer, size, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
}

// Hàm lấy tên tiến trình hiện tại
std::string GetProcessName(DWORD processID) {
    char process_name[MAX_PATH] = "<unknown>";
    HANDLE process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (process_handle) {
        HMODULE module;
        DWORD cb_needed;
        if (EnumProcessModules(process_handle, &module, sizeof(module), &cb_needed)) {
            GetModuleFileNameExA(process_handle, module, process_name, sizeof(process_name));
        }
        CloseHandle(process_handle);
    }
    return std::string(process_name);
}

int main() {
    const char* path = "C:\\Users\\buivu\\Documents";
    printf("watching %s for changes...\n", path);

    // Chuyển đổi đường dẫn từ UTF-8 sang wchar_t
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);
    wchar_t* wpath = new wchar_t[size_needed];
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, size_needed);

    // Mở thư mục để theo dõi thay đổi
    HANDLE file = CreateFile(wpath,
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL);
    assert(file != INVALID_HANDLE_VALUE);

    // Cấu trúc OVERLAPPED để sử dụng cho hàm ReadDirectoryChangesW
    OVERLAPPED overlapped;
    overlapped.hEvent = CreateEvent(NULL, FALSE, 0, NULL);

    uint8_t change_buf[1024];
    BOOL success = ReadDirectoryChangesW(
        file, change_buf, 1024, TRUE,
        FILE_NOTIFY_CHANGE_FILE_NAME |
        FILE_NOTIFY_CHANGE_DIR_NAME |
        FILE_NOTIFY_CHANGE_LAST_WRITE,
        NULL, &overlapped, NULL);

    // Mở file log để ghi lại các thay đổi
    std::ofstream log_file("file_changes.log");

    while (true) {
        DWORD result = WaitForSingleObject(overlapped.hEvent, 0);

        if (result == WAIT_OBJECT_0) {
            DWORD bytes_transferred;
            GetOverlappedResult(file, &overlapped, &bytes_transferred, FALSE);

            // Xử lý sự kiện thay đổi
            FILE_NOTIFY_INFORMATION* event = (FILE_NOTIFY_INFORMATION*)change_buf;
            for (;;) {
                DWORD name_len = event->FileNameLength / sizeof(wchar_t);
                char time_buffer[64];
                GetFormattedTime(time_buffer, sizeof(time_buffer));

                // Lấy ID tiến trình
                DWORD process_id = GetCurrentProcessId();

                // Lấy tên tiến trình
                std::string process_name = GetProcessName(process_id);

                char process_id_str[11];
                sprintf_s(process_id_str, "%lu", process_id);

                std::wstring ws(event->FileName, name_len);
                std::string log_entry = std::string(time_buffer) +
                    ", PID: " + process_id_str +
                    ", Process: " + process_name +
                    ", ";

                // Xác định hành động đã xảy ra và thêm vào log
                switch (event->Action) {
                case FILE_ACTION_ADDED:
                    log_entry += "Added, " + std::string(ws.begin(), ws.end());
                    break;
                case FILE_ACTION_REMOVED:
                    log_entry += "Removed, " + std::string(ws.begin(), ws.end());
                    break;
                case FILE_ACTION_MODIFIED:
                    log_entry += "Modified, " + std::string(ws.begin(), ws.end());
                    break;
                case FILE_ACTION_RENAMED_OLD_NAME:
                    log_entry += "Renamed from, " + std::string(ws.begin(), ws.end());
                    break;
                case FILE_ACTION_RENAMED_NEW_NAME:
                    log_entry += "Renamed to, " + std::string(ws.begin(), ws.end());
                    break;
                default:
                    log_entry += "Unknown action!";
                    break;
                }

                // In thông tin log ra console và ghi vào file log
                wprintf(L"%hs\n", log_entry.c_str());
                log_file << log_entry << std::endl;

                // Kiểm tra xem còn sự kiện nào cần xử lý không
                if (event->NextEntryOffset) {
                    *((uint8_t**)&event) += event->NextEntryOffset;
                }
                else {
                    break;
                }
            }

            // Xếp hàng sự kiện tiếp theo
            success = ReadDirectoryChangesW(
                file, change_buf, 1024, TRUE,
                FILE_NOTIFY_CHANGE_FILE_NAME |
                FILE_NOTIFY_CHANGE_DIR_NAME |
                FILE_NOTIFY_CHANGE_LAST_WRITE,
                NULL, &overlapped, NULL);
        }
    }

    // Đóng file log
    log_file.close();

    // Dọn dẹp tài nguyên
    CloseHandle(file);
    CloseHandle(overlapped.hEvent);
    delete[] wpath;

    return 0;
}
