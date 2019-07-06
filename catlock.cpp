/*
 * Copyright (c) 2019, Fritz Elfert
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <xcb/dpms.h>
#include <xcb/xcb_keysyms.h>
#include <X11/Xlib.h>
#include <popt.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <cstdlib>

using namespace std;

/* cat icon grabbed from oneko (public domain) */
static uint8_t cat_src_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x10, 0x10, 0x00, 0x00, 0x28, 0x28, 0x00, 0x00, 0x48, 0x24, 0x00,
    0x00, 0x44, 0x44, 0x00, 0x00, 0x84, 0x42, 0x00, 0x00, 0x82, 0x83, 0x00,
    0x00, 0x02, 0x80, 0x00, 0x00, 0x22, 0x88, 0x00, 0x00, 0x22, 0x88, 0x00,
    0x00, 0x22, 0x88, 0x00, 0x00, 0x02, 0x80, 0x00, 0x00, 0x3a, 0xb9, 0x00,
    0x00, 0x04, 0x40, 0x00, 0x00, 0x08, 0x20, 0x00, 0x00, 0x70, 0x1c, 0x00,
    0x00, 0x40, 0x04, 0x00, 0x00, 0x20, 0x08, 0x00, 0x00, 0x10, 0x10, 0x00,
    0x00, 0x08, 0x20, 0x00, 0x00, 0x0b, 0xa0, 0x01, 0x80, 0x0c, 0x61, 0x02,
    0x40, 0x18, 0x31, 0x04, 0x40, 0x10, 0x11, 0x04, 0xc0, 0x11, 0x11, 0x7f,
    0x60, 0x90, 0x13, 0x84, 0xe0, 0xff, 0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static uint8_t cat_mask_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x10, 0x10, 0x00, 0x00, 0x38, 0x38, 0x00, 0x00, 0x78, 0x3c, 0x00,
    0x00, 0x7c, 0x7c, 0x00, 0x00, 0xfc, 0x7e, 0x00, 0x00, 0xfe, 0xff, 0x00,
    0x00, 0xfe, 0xff, 0x00, 0x00, 0xfe, 0xff, 0x00, 0x00, 0xfe, 0xff, 0x00,
    0x00, 0xfe, 0xff, 0x00, 0x00, 0xfe, 0xff, 0x00, 0x00, 0xfe, 0xff, 0x00,
    0x00, 0xfc, 0x7f, 0x00, 0x00, 0xf8, 0x3f, 0x00, 0x00, 0xf0, 0x1f, 0x00,
    0x00, 0xc0, 0x07, 0x00, 0x00, 0xe0, 0x0f, 0x00, 0x00, 0xf0, 0x1f, 0x00,
    0x00, 0xf8, 0x3f, 0x00, 0x00, 0xfb, 0xbf, 0x01, 0x80, 0xff, 0xff, 0x03,
    0xc0, 0xff, 0xff, 0x07, 0xc0, 0xff, 0xff, 0x07, 0xc0, 0xff, 0xff, 0x7f,
    0xe0, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const string csr_bg_color("steelblue3");
static const string csr_fg_color("grey25");

struct color {
    uint16_t r, g, b;
};
typedef struct color color_t;

static color_t alloc_named_color(xcb_connection_t *conn,  xcb_colormap_t cmap,
        const string &cname)
{
    xcb_alloc_named_color_cookie_t c = xcb_alloc_named_color(conn, cmap,
            cname.length(), cname.c_str());
    xcb_generic_error_t *e;
    xcb_alloc_named_color_reply_t *r = xcb_alloc_named_color_reply(conn, c, &e);
    if (!r) {
        cerr << "Could not allocate named color: X11 error " << e->error_code << endl;
        xcb_disconnect(conn);
        exit(-1);
    }
    struct color ret;
    ret.r = r->visual_red;
    ret.g = r->visual_green;
    ret.b = r->visual_blue;
    free(r);
    return ret;
}

static xcb_cursor_t create_cursor(xcb_connection_t *conn, xcb_pixmap_t src, xcb_pixmap_t mask,
        color_t fg, color_t bg, uint16_t hot_x, uint16_t hot_y)
{
    xcb_cursor_t cid = xcb_generate_id(conn);
    xcb_void_cookie_t c = xcb_create_cursor_checked(conn, cid, src, mask,
            fg.r, fg.g, fg.b, bg.r, bg.g, bg.g, hot_x, hot_y);
    xcb_generic_error_t *e = xcb_request_check(conn, c);
    if (e) {
        cerr << "Could not create cursor: X11 error " << e->error_code << endl;
        xcb_disconnect(conn);
        exit(-1);
    }
    return cid;
}

static uint8_t grab_pointer(xcb_connection_t *conn, uint8_t owner_events, xcb_window_t win,
        uint16_t event_mask, uint8_t ptrmode, uint8_t kbdmode, xcb_window_t confine_to,
        xcb_cursor_t csr, xcb_timestamp_t time)
{
    xcb_grab_pointer_cookie_t c = xcb_grab_pointer(conn, owner_events, win, event_mask,
            ptrmode, kbdmode, confine_to, csr, time);
    xcb_generic_error_t *e;
    xcb_grab_pointer_reply_t *r = xcb_grab_pointer_reply(conn, c, &e);
    if (!r) {
        cerr << "Could not grab pointer: X11 error " << e->error_code << endl;
        xcb_disconnect(conn);
        exit(-1);
    }
    uint8_t ret = r->status;
    free(r);
    return ret;
}

static uint8_t grab_keyboard(xcb_connection_t *conn, uint8_t owner_events, xcb_window_t win,
        xcb_timestamp_t time, uint8_t ptrmode, uint8_t kbdmode)
{
    xcb_grab_keyboard_cookie_t c = xcb_grab_keyboard(conn, owner_events, win, time,
            ptrmode, kbdmode);
    xcb_generic_error_t *e;
    xcb_grab_keyboard_reply_t *r = xcb_grab_keyboard_reply(conn, c, &e);
    if (!r) {
        cerr << "Could not grab keyboard: X11 error " << e->error_code << endl;
        xcb_disconnect(conn);
        exit(-1);
    }
    uint8_t ret = r->status;
    free(r);
    return ret;
}

static string nullable(const char *in)
{
    return string(nullptr == in ? "null" : in);
}

static void debug_key(xcb_key_press_event_t *evt, xcb_key_symbols_t *ksyms)
{
    cout << "code=0x" << hex << setw(2) << setfill('0')
        << static_cast<int>(evt->detail) << " state=0x" << setw(4)
        << evt->state << " syms:";
    for (int i = 0; 8 > i; i++) {
        xcb_keysym_t sym = xcb_key_press_lookup_keysym(ksyms, evt, i);
        cout << " 0x" << setw(8) << sym << "/"
            << nullable(XKeysymToString(sym));
    }
    cout << endl;
}

static map<string, string> ksym_rename = {
    {"Control_L", "Ctrl"},
    {"Control_R", "Ctrl"},
    {"Shift_L", "Shift"},
    {"Shift_R", "Shift"},
    {"Alt_L", "Alt"},
    {"Alt_R", "Alt"},
    {"Super_L", "Meta"},
    {"Super_R", "Meta"},
    {"Caps_Lock", "CapsLock"},
    {"Scroll_Lock", "ScrollLock"},
    {"Num_Lock", "NumLock"},
};

static string generic(const string &in)
{
    string ret(in);
    for (auto &kv : ksym_rename) {
        if (0 == kv.first.compare(ret)) {
            ret.assign(kv.second);
            break;
        }
    }
    return ret;
}

static string keysym(xcb_key_press_event_t *evt, xcb_key_symbols_t *ksyms)
{
    return generic(nullable(XKeysymToString(xcb_key_press_lookup_keysym(ksyms, evt, 0))));
}

static string modifiers(uint16_t state) {
    string ret;
    if (state & 0x01) {
        ret.append("Shift+");
    }
    if (state & 0x04) {
        ret.append("Ctrl+");
    }
    if (state & 0x08) {
        ret.append("Alt+");
    }
    if (state & 0x40) {
        ret.append("Meta+");
    }
    return ret;
}

static string keystr(xcb_key_press_event_t *evt, xcb_key_symbols_t *ksyms)
{
    return modifiers(evt->state).append(keysym(evt, ksyms));
}

static vector<string> split(const string &in, const char delim)
{
    stringstream ss(in);
    string item;
    vector<string> ret;
    while (getline(ss, item, delim)) {
        ret.push_back(move(item));
    }
    return ret;
}

static void waitfor_keyrelease(xcb_connection_t *conn, const string &kstr, xcb_key_symbols_t *ksyms)
{
    vector<string> v(split(kstr, '+'));
    xcb_generic_event_t *event;
    while (!v.empty() && (event = xcb_wait_for_event(conn))) {
        if (XCB_KEY_RELEASE == event->response_type) {
            xcb_key_release_event_t *kev = reinterpret_cast<xcb_key_release_event_t *>(event);
            const string ksym(keysym(kev, ksyms));
            vector<string>::iterator it = find(v.begin(), v.end(), ksym);
            if (it != v.end()) {
                v.erase(it);
            }
        }
    }
}

/**
 * Validate and normalize (reorder modifiers) a shortcut.
 */
static bool validate_shortcut(string &shortcut, xcb_key_symbols_t *ksyms)
{
    uint16_t state = 0;
    string plain;
    vector<string> v(split(shortcut, '+'));
    vector<string>::iterator it = v.begin();
    bool wrong = false;
    while (it != v.end()) {
        bool found = false;
        for (int code = 0; 255 >= code; code++) {
            string ksym(generic(nullable(XKeysymToString(xcb_key_symbols_get_keysym(ksyms, code, 0)))));
            if (0 == it->compare(ksym)) {
                found = true;
                if (1 == v.size()) {
                    plain.assign(*it);
                } else {
                    if (0 == it->compare("Shift")) {
                        state |= 0x01;
                    } else if (0 == it->compare("Ctrl")) {
                        state |= 0x04;
                    } else if (0 == it->compare("Alt")) {
                        state |= 0x08;
                    } else if (0 == it->compare("Meta")) {
                        state |= 0x40;
                    } else {
                        wrong = true;
                    }
                }
            }
        }
        if (found) {
            it = v.erase(it);
        } else {
            ++it;
        }
    }
    if (wrong || plain.empty() || !v.empty()) {
        cerr << '"' << shortcut << "\" is not a valid shortcut" << endl;
        return false;
    }
    shortcut.assign(modifiers(state).append(plain));
    return true;
}

int main(int argc, const char **argv)
{
    char *display = nullptr;
    char *scut = const_cast<char *>("Ctrl+Alt+KP_Enter");
    char *oscut = const_cast<char *>("Meta+l");
    int vprint = 0;
    int print = 0;
    int dbg = 0;

    struct poptOption po[] = {
        {"display", '\0', POPT_ARG_STRING, &display, 0, "X11 display name", "DisplayName"},
        {"shortcut", 's', POPT_ARG_STRING, &scut, 0, "Shortcut to end the program", "KeysymString"},
        {"oshortcut", 'o', POPT_ARG_STRING, &oscut, 0, "Shortcut to switch off the screen", "KeysymString"},
        {"debug", 'd', POPT_ARG_NONE, &dbg, 0, "Enable debug output", ""},
        {"print", 'p', POPT_ARG_NONE, &print, 0, "Print keysym strings", ""},
        {"version", '\0', POPT_ARG_NONE, &vprint, 0, "Print version and exit", ""},
        POPT_AUTOHELP
        POPT_TABLEEND
    };
    // pc is the context for all popt-related functions
    poptContext pc = poptGetContext(nullptr, argc, argv, po, 0);
    int rc;
    while (0 <= (rc = poptGetNextOpt(pc))) {
    }
    if (-1 != rc) {
        cerr << poptBadOption(pc, POPT_BADOPTION_NOALIAS) << ": " << poptStrerror(rc) << endl;
        exit(1);
    }

    if (vprint) {
        cout << "catlock " << VERSION << " © 2019 Fritz Elfert" << endl;
        exit(0);
    }

    int nrscreens;
    xcb_connection_t *conn = xcb_connect(display, &nrscreens);
    if (xcb_connection_has_error(conn)) {
        cerr << "Could not open display" << endl;
        exit(-1);
    }
    const xcb_setup_t *setup = xcb_get_setup(conn);
    xcb_key_symbols_t *ksyms = xcb_key_symbols_alloc(conn);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);

    string shortcut(scut);
    string oshortcut(oscut);
    if (!validate_shortcut(shortcut, ksyms)) {
        xcb_disconnect(conn);
        exit(1);
    }
    if (!validate_shortcut(oshortcut, ksyms)) {
        xcb_disconnect(conn);
        exit(1);
    }
    if (dbg) {
        cout << "Termination shortcut: " << shortcut << endl;
        cout << "DPMS shortcut: " << oshortcut << endl;
    }

    for (; iter.rem; --nrscreens, xcb_screen_next(&iter)) {
        if (nrscreens == 0) {
            const xcb_screen_t *screen = iter.data;
            const xcb_window_t win = xcb_generate_id(conn);
            uint32_t values[2] = {1, XCB_EVENT_MASK_KEY_PRESS};
            xcb_create_window(conn, XCB_COPY_FROM_PARENT, win, screen->root,
                    0, 0, 1, 1, 0, XCB_WINDOW_CLASS_INPUT_ONLY,
                    XCB_COPY_FROM_PARENT,
                    XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK, values);
            const xcb_pixmap_t csr_src = xcb_create_pixmap_from_bitmap_data(conn, win, cat_src_bits,
                    32, 32, 1, 0, 0, nullptr);
            const xcb_pixmap_t csr_mask = xcb_create_pixmap_from_bitmap_data(conn, win, cat_mask_bits,
                    32, 32, 1, 0, 0, nullptr);
            const color_t csr_fg = alloc_named_color(conn, screen->default_colormap, csr_fg_color);
            const color_t csr_bg = alloc_named_color(conn, screen->default_colormap, csr_bg_color);
            const xcb_cursor_t cursor = create_cursor(conn, csr_src, csr_mask, csr_fg, csr_bg, 16, 16);

            xcb_map_window(conn, win);
            int tvt, grab_ok;
            struct timeval tv;
            for (tvt = 0, grab_ok = 0; 100 > tvt; tvt++) {
                if (XCB_GRAB_STATUS_SUCCESS == grab_keyboard(conn, 0, win, XCB_CURRENT_TIME,
                            XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC)) {
                    grab_ok = 1;
                    break;
                }
                /* grab failed: wait .01s */
                tv.tv_sec = 0;
                tv.tv_usec = 10000;
                select(1, nullptr, nullptr, nullptr, &tv);
            }
            if (!grab_ok) {
                cerr << "Keyboard grab timed out" << endl;
                xcb_disconnect(conn);
                exit(-1);
            }
            for (tvt = 0, grab_ok = 0; 100 > tvt; tvt++) {
                if (XCB_GRAB_STATUS_SUCCESS == grab_pointer(conn, 0, win, 0,
                            XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_WINDOW_NONE,
                            cursor, XCB_CURRENT_TIME)) {
                    grab_ok = 1;
                    break;
                    /* grab failed: wait .01s */
                    tv.tv_sec = 0;
                    tv.tv_usec = 10000;
                    select(1, nullptr, nullptr, nullptr, &tv);
                }
            }
            if (!grab_ok) {
                cerr << "Pointer grab timed out" << endl;
                xcb_disconnect(conn);
                exit(-1);
            }
            xcb_flush(conn);
            xcb_generic_event_t *event;
            while ((event = xcb_wait_for_event(conn))) {
                if (XCB_KEY_PRESS == event->response_type) {
                    xcb_key_press_event_t *kev = reinterpret_cast<xcb_key_press_event_t *>(event);
                    if (dbg) {
                        debug_key(kev, ksyms);
                    }
                    string kstr(keystr(kev, ksyms));
                    if (print) {
                        cout << kstr << endl;
                    }
                    if (0 == kstr.compare(shortcut)) {
                        break;
                    }
                    if (0 == kstr.compare(oshortcut)) {
                        // Wait until all involved key(s) are released. Otherwise,
                        // any key release event could trigger a DPMS wakeup.
                        waitfor_keyrelease(conn, kstr, ksyms);
                        xcb_void_cookie_t c = xcb_dpms_force_level_checked(conn, XCB_DPMS_DPMS_MODE_OFF);
                        xcb_generic_error_t *e = xcb_request_check(conn, c);
                        if (e) {
                            cerr << "Could not switch off display: X11 error " << e->error_code << endl;
                            xcb_disconnect(conn);
                            exit(-1);
                        }
                    }
                }
            }
            break;
        }
    }
}
