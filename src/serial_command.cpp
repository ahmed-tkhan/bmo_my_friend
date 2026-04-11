#include "serial_command.h"

SerialCommand::SerialCommand()
    : stream_(nullptr) {}

void SerialCommand::begin(Stream* stream) {
    stream_ = stream;
    lineBuf_.reserve(128);
}

void SerialCommand::registerCommand(const String& name, Handler cb) {
    Cmd c; c.name = toUpperCopy(name); c.cb = cb;
    cmds_.push_back(c);
}

String SerialCommand::toUpperCopy(const String& s) {
    String t = s;
    t.toUpperCase();
    return t;
}

void SerialCommand::reply(const String& s) {
    if (!stream_) return;
    stream_->println(s);
}

void SerialCommand::process() {
    if (!stream_) return;
    while (stream_->available()) {
        int c = stream_->read();
        if (c == -1) break;
        if (c == '\r') continue;
        if (c == '\n') {
            String line = lineBuf_;
            lineBuf_.clear();
            line.trim();
            if (line.length() == 0) continue;
            String up = toUpperCopy(line);
            // Extract command and args
            int sp = up.indexOf(' ');
            String cmd = up;
            String args = "";
            if (sp >= 0) {
                cmd = up.substring(0, sp);
                args = line.substring(sp + 1);
                args.trim();
            }
            bool handled = false;
            for (const auto &e : cmds_) {
                if (e.name == cmd) {
                    e.cb(args);
                    handled = true;
                    break;
                }
            }
            if (!handled) {
                // try prefix match for names like EMOTION.LIST / EMOTION.SET
                for (const auto &e : cmds_) {
                    if (e.name.startsWith(cmd)) {
                        e.cb(args);
                        handled = true;
                        break;
                    }
                }
            }
            if (!handled) {
                reply(String("ERR Unknown command: ") + line);
            }
        } else {
            lineBuf_ += (char)c;
            // prevent runaway buffer
            if (lineBuf_.length() > 512) lineBuf_.remove(0, lineBuf_.length() - 512);
        }
    }
}
