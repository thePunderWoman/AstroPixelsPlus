#pragma once
#ifdef USE_WIFI_WEB

// =============================================================================
// Web Console — ring buffer + helpers used by WebPages.h WAPI endpoints
// =============================================================================

#define WEB_CONSOLE_LINES 80

static struct {
    String lines[WEB_CONSOLE_LINES];
    int    head  = 0;   // next write slot
    int    total = 0;   // monotonic line count (client uses as sequence cursor)
} sWebConsole;

static void webConsoleLog(const String& msg)
{
    sWebConsole.lines[sWebConsole.head] = msg;
    sWebConsole.head = (sWebConsole.head + 1) % WEB_CONSOLE_LINES;
    sWebConsole.total++;
}

static void webConsoleLog(const char* msg)
{
    webConsoleLog(String(msg));
}

// Simple URL-decode (%XX and + → space)
static String webUrlDecode(const String& s)
{
    String out;
    out.reserve(s.length());
    for (int i = 0; i < (int)s.length(); i++) {
        if (s[i] == '+') {
            out += ' ';
        } else if (s[i] == '%' && i + 2 < (int)s.length()) {
            char h[3] = { s[i+1], s[i+2], '\0' };
            out += (char)strtol(h, nullptr, 16);
            i += 2;
        } else {
            out += s[i];
        }
    }
    return out;
}

// Returns JSON { "seq": N, "lines": [...] } for lines written since `since`
static String webConsoleJSON(int since)
{
    int avail = sWebConsole.total - since;
    if (avail < 0) avail = 0;
    if (avail > WEB_CONSOLE_LINES) avail = WEB_CONSOLE_LINES;

    String j = "{\"seq\":";
    j += sWebConsole.total;
    j += ",\"lines\":[";
    for (int i = 0; i < avail; i++) {
        if (i) j += ',';
        int slot = ((sWebConsole.head - avail + i) % WEB_CONSOLE_LINES + WEB_CONSOLE_LINES) % WEB_CONSOLE_LINES;
        j += '"';
        String esc = sWebConsole.lines[slot];
        esc.replace("\\", "\\\\");
        esc.replace("\"", "\\\"");
        j += esc;
        j += '"';
    }
    j += "]}";
    return j;
}

#endif // USE_WIFI_WEB
