#!/usr/bin/env bash
set -euo pipefail

# rabbitGram Linux Installer & Uninstaller Script
APP_NAME="rabbitGram"
APP_WM_CLASS="rabbitGram"
APP_DATA_DIR_NAME="rabbitGramDesktop"
APP_GENERIC_NAME="Messaging Client"
APP_COMMENT="rabbitGram Desktop Messaging Client"
DESKTOP_FILE_NAME="rabbitGram.desktop"
ICON_NAME="rabbitGram"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

print_help() {
    cat << EOF
Usage: $0 [OPTIONS] [PATH_TO_BINARY]

Installs rabbitGram with desktop entry, icons, and 'tg://' protocol registration.

Options:
  --user            Install for the current user only (default: ~/.local)
  --system          Install system-wide (requires root/sudo: /usr/local)
  --uninstall       Uninstall rabbitGram (removes binary, icons, desktop entry)
  --strip           Strip debugging symbols from copied binary (default: enabled)
  --no-strip        Keep debugging symbols without stripping
  -h, --help        Show this help message

If PATH_TO_BINARY is omitted, the installer automatically checks:
  1. ./out/Release/rabbitGram
  2. ./rabbitGram
EOF
}

INSTALL_MODE="user"
DO_UNINSTALL=false
DO_STRIP=true
BINARY_ARG=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        --user)
            INSTALL_MODE="user"
            shift
            ;;
        --system)
            INSTALL_MODE="system"
            shift
            ;;
        --uninstall)
            DO_UNINSTALL=true
            shift
            ;;
        --strip)
            DO_STRIP=true
            shift
            ;;
        --no-strip)
            DO_STRIP=false
            shift
            ;;
        -h|--help)
            print_help
            exit 0
            ;;
        *)
            if [[ -z "$BINARY_ARG" ]]; then
                BINARY_ARG="$1"
                shift
            else
                echo "Error: Unknown argument '$1'" >&2
                print_help
                exit 1
            fi
            ;;
    esac
done

# Define target paths
if [[ "$INSTALL_MODE" == "system" ]]; then
    if [[ $EUID -ne 0 ]]; then
        echo "Error: System-wide installation requires root privileges (run with sudo)." >&2
        exit 1
    fi
    BIN_DIR="/usr/local/bin"
    APP_DIR="/usr/share/applications"
    ICON_BASE_DIR="/usr/share/icons/hicolor"
    AUTOSTART_DIR="/etc/xdg/autostart"
else
    if [[ $EUID -eq 0 ]]; then
        echo "Error: Running user installation (--user) as root writes files to /root/.local/." >&2
        echo "       To install for your normal user account, run without sudo: ./install-rabbitgram.sh" >&2
        echo "       To install system-wide, use: sudo ./install-rabbitgram.sh --system" >&2
        exit 1
    fi
    BIN_DIR="${XDG_BIN_HOME:-${HOME}/.local/bin}"
    XDG_DATA_HOME="${XDG_DATA_HOME:-${HOME}/.local/share}"
    APP_DIR="${XDG_DATA_HOME}/applications"
    ICON_BASE_DIR="${XDG_DATA_HOME}/icons/hicolor"
    AUTOSTART_DIR="${XDG_CONFIG_HOME:-${HOME}/.config}/autostart"
fi

TARGET_BIN="${BIN_DIR}/${APP_NAME}"
TARGET_UPDATER="${BIN_DIR}/Updater"
TARGET_DESKTOP="${APP_DIR}/${DESKTOP_FILE_NAME}"

# Handle uninstallation
if [[ "$DO_UNINSTALL" = true ]]; then
    echo "==> Uninstalling ${APP_NAME} (${INSTALL_MODE} mode)..."
    
    if [[ -f "$TARGET_BIN" ]]; then
        echo "Removing binary: $TARGET_BIN"
        rm -f "$TARGET_BIN"
    fi

    if [[ -f "$TARGET_UPDATER" ]]; then
        echo "Removing updater: $TARGET_UPDATER"
        rm -f "$TARGET_UPDATER"
    fi

    if [[ -f "$TARGET_DESKTOP" ]]; then
        echo "Removing desktop entry: $TARGET_DESKTOP"
        rm -f "$TARGET_DESKTOP"
    fi

    if [[ -f "${AUTOSTART_DIR}/${DESKTOP_FILE_NAME}" ]]; then
        echo "Removing autostart entry: ${AUTOSTART_DIR}/${DESKTOP_FILE_NAME}"
        rm -f "${AUTOSTART_DIR}/${DESKTOP_FILE_NAME}"
    fi

    rm -f "${APP_DIR}"/rabbitGram._*.desktop 2>/dev/null || true
    if [[ "$INSTALL_MODE" == "user" ]]; then
        rm -f "${XDG_DATA_HOME}/dbus-1/services"/rabbitGram*.service 2>/dev/null || true
    fi

    echo "Removing icons..."
    for size in 16 32 48 64 128 256 512; do
        target_icon="${ICON_BASE_DIR}/${size}x${size}/apps/${ICON_NAME}.png"
        if [[ -f "$target_icon" ]]; then
            rm -f "$target_icon"
        fi
    done

    for sym in "${ICON_NAME}-symbolic.svg" "${ICON_NAME}-attention-symbolic.svg" "${ICON_NAME}-mute-symbolic.svg"; do
        target_sym="${ICON_BASE_DIR}/symbolic/apps/${sym}"
        if [[ -f "$target_sym" ]]; then
            rm -f "$target_sym"
        fi
    done

    echo "Refreshing desktop database..."
    if command -v update-desktop-database >/dev/null 2>&1; then
        update-desktop-database "$APP_DIR" 2>/dev/null || true
    fi
    if command -v gtk-update-icon-cache >/dev/null 2>&1; then
        gtk-update-icon-cache -f -t "$ICON_BASE_DIR" 2>/dev/null || true
    fi

    echo "==> Uninstallation complete."
    exit 0
fi

# Locate binary
SRC_BIN=""
if [[ -n "$BINARY_ARG" ]]; then
    SRC_BIN="$BINARY_ARG"
elif [[ -f "${SCRIPT_DIR}/out/Release/rabbitGram" ]]; then
    SRC_BIN="${SCRIPT_DIR}/out/Release/rabbitGram"
elif [[ -f "${SCRIPT_DIR}/rabbitGram" ]]; then
    SRC_BIN="${SCRIPT_DIR}/rabbitGram"
elif [[ -f "./rabbitGram" ]]; then
    SRC_BIN="./rabbitGram"
fi

if [[ -z "$SRC_BIN" || ! -f "$SRC_BIN" ]]; then
    echo "Error: rabbitGram binary not found." >&2
    echo "Please provide the path to the rabbitGram executable, e.g.:" >&2
    echo "  $0 ./out/Release/rabbitGram" >&2
    exit 1
fi

echo "==> Installing ${APP_NAME} (${INSTALL_MODE} mode)"
echo "    Source binary: $SRC_BIN"
echo "    Destination:   $TARGET_BIN"

# Create directories
mkdir -p "$BIN_DIR" "$APP_DIR"

# Copy binary avoiding ETXTBSY if application is currently open
if [[ -f "$TARGET_BIN" ]] && [[ "$(readlink -f "$SRC_BIN" 2>/dev/null || echo "")" == "$(readlink -f "$TARGET_BIN" 2>/dev/null || echo "")" ]]; then
    echo "Source and destination binary are identical ($TARGET_BIN). Skipping binary copy."
else
    echo "Installing binary to ${TARGET_BIN}..."
    install -m 755 "$SRC_BIN" "$TARGET_BIN" 2>/dev/null || {
        rm -f "$TARGET_BIN"
        cp -f "$SRC_BIN" "$TARGET_BIN"
        chmod 755 "$TARGET_BIN"
    }
fi

# Locate and install optional Updater (if built alongside rabbitGram)
SRC_UPDATER=""
if [[ -f "$(dirname "$SRC_BIN")/Updater" ]]; then
    SRC_UPDATER="$(dirname "$SRC_BIN")/Updater"
elif [[ -f "${SCRIPT_DIR}/out/Release/Updater" ]]; then
    SRC_UPDATER="${SCRIPT_DIR}/out/Release/Updater"
elif [[ -f "${SCRIPT_DIR}/Updater" ]]; then
    SRC_UPDATER="${SCRIPT_DIR}/Updater"
fi

if [[ -n "$SRC_UPDATER" && -f "$SRC_UPDATER" ]]; then
    if [[ -f "$TARGET_UPDATER" ]] && [[ "$(readlink -f "$SRC_UPDATER" 2>/dev/null || echo "")" == "$(readlink -f "$TARGET_UPDATER" 2>/dev/null || echo "")" ]]; then
        echo "Source and destination Updater are identical ($TARGET_UPDATER). Skipping copy."
    else
        echo "Installing Updater to ${TARGET_UPDATER}..."
        install -m 755 "$SRC_UPDATER" "$TARGET_UPDATER" 2>/dev/null || {
            rm -f "$TARGET_UPDATER"
            cp -f "$SRC_UPDATER" "$TARGET_UPDATER"
            chmod 755 "$TARGET_UPDATER"
        }
    fi
fi

if [[ "$DO_STRIP" = true ]]; then
    if command -v strip >/dev/null 2>&1; then
        echo "Stripping debug symbols from ${TARGET_BIN}..."
        strip "$TARGET_BIN"
        if [[ -n "$SRC_UPDATER" && -f "$TARGET_UPDATER" ]]; then
            echo "Stripping debug symbols from ${TARGET_UPDATER}..."
            strip "$TARGET_UPDATER"
        fi
    else
        echo "Warning: strip command not found, skipping stripping." >&2
    fi
fi

# Install icons
echo "Installing application icons..."
if [[ -d "${SCRIPT_DIR}/icons/art" ]]; then
    ART_DIR="${SCRIPT_DIR}/icons/art"
elif [[ -d "${SCRIPT_DIR}/icons" && -f "${SCRIPT_DIR}/icons/icon256.png" ]]; then
    ART_DIR="${SCRIPT_DIR}/icons"
elif [[ -d "${SCRIPT_DIR}/Telegram/Resources/art" ]]; then
    ART_DIR="${SCRIPT_DIR}/Telegram/Resources/art"
else
    ART_DIR=""
fi

if [[ -n "$ART_DIR" ]]; then
    for size in 16 32 48 64 128 256 512; do
        src_icon="${ART_DIR}/icon${size}.png"
        dest_dir="${ICON_BASE_DIR}/${size}x${size}/apps"
        if [[ -f "$src_icon" ]]; then
            mkdir -p "$dest_dir"
            cp -f "$src_icon" "${dest_dir}/${ICON_NAME}.png"
        fi
    done
fi

# Install tray monochrome icons if available
if [[ -d "${SCRIPT_DIR}/icons/tray" ]]; then
    TRAY_DIR="${SCRIPT_DIR}/icons/tray"
elif [[ -d "${SCRIPT_DIR}/Telegram/Resources/icons" ]]; then
    TRAY_DIR="${SCRIPT_DIR}/Telegram/Resources/icons"
else
    TRAY_DIR=""
fi

SYM_DIR="${ICON_BASE_DIR}/symbolic/apps"
if [[ -n "$TRAY_DIR" && -d "$TRAY_DIR" ]]; then
    mkdir -p "$SYM_DIR"
    [[ -f "${TRAY_DIR}/tray_monochrome.svg" ]] && cp -f "${TRAY_DIR}/tray_monochrome.svg" "${SYM_DIR}/${ICON_NAME}-symbolic.svg"
    [[ -f "${TRAY_DIR}/tray_monochrome_attention.svg" ]] && cp -f "${TRAY_DIR}/tray_monochrome_attention.svg" "${SYM_DIR}/${ICON_NAME}-attention-symbolic.svg"
    [[ -f "${TRAY_DIR}/tray_monochrome_mute.svg" ]] && cp -f "${TRAY_DIR}/tray_monochrome_mute.svg" "${SYM_DIR}/${ICON_NAME}-mute-symbolic.svg"
fi

# Clean up any legacy hashed rabbitGram desktop files or invalid service files from previous launches
rm -f "${APP_DIR}"/rabbitGram._*.desktop 2>/dev/null || true
if [[ "$INSTALL_MODE" == "user" ]]; then
    rm -f "${XDG_DATA_HOME}/dbus-1/services"/rabbitGram*.service 2>/dev/null || true
fi

# Generate .desktop file
echo "Creating desktop entry at ${TARGET_DESKTOP}..."
cat << EOF > "$TARGET_DESKTOP"
[Desktop Entry]
Version=1.5
Type=Application
Name=${APP_NAME}
GenericName=${APP_GENERIC_NAME}
Comment=${APP_COMMENT}
TryExec=${TARGET_BIN}
# env DESKTOPINTEGRATION=0 prevents internal InstallLauncher() from re-generating launcher entries on startup
Exec=env DESKTOPINTEGRATION=0 ${TARGET_BIN} -- %U
Icon=${ICON_NAME}
Terminal=false
StartupWMClass=${APP_WM_CLASS}
Categories=Chat;Network;InstantMessaging;Qt;
MimeType=x-scheme-handler/tg;x-scheme-handler/tonsite;
Keywords=tg;chat;im;messaging;messenger;sms;tdesktop;rabbitgram;rabbitGram;
Actions=quit;
SingleMainWindow=true
X-GNOME-UsesNotifications=true
X-GNOME-SingleWindow=true

[Desktop Action quit]
Exec=${TARGET_BIN} -quit
Name=Quit rabbitGram
Icon=application-exit
EOF

chmod +x "$TARGET_DESKTOP"

# Refresh desktop & icon caches
echo "Updating desktop database and icon cache..."
if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database "$APP_DIR" 2>/dev/null || true
fi
if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -f -t "$ICON_BASE_DIR" 2>/dev/null || true
fi

# Register default tg:// and tonsite:// scheme handlers if user mode
if [[ "$INSTALL_MODE" == "user" ]] && command -v xdg-mime >/dev/null 2>&1; then
    echo "Associating 'tg://' and 'tonsite://' links with ${DESKTOP_FILE_NAME}..."
    xdg-mime default "${DESKTOP_FILE_NAME}" x-scheme-handler/tg || true
    xdg-mime default "${DESKTOP_FILE_NAME}" x-scheme-handler/tonsite || true
fi

echo ""
echo "==> Installation complete!"
echo "    App Launcher: ${APP_NAME} is now available in your application menu."
echo "    Command-line: Run '${APP_NAME}' directly (ensure ${BIN_DIR} is in your PATH)."
echo "    Session Data: tdata location is ~/.local/share/${APP_DATA_DIR_NAME}/tdata"
