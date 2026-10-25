/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "rabbit/tg_ws_proxy/tg_ws_proxy_constants.h"

#include <QtCore/QByteArray>
#include <QtCore/QtEndian>
#include <vector>

namespace Rabbit::TgWsProxy {

class MsgSplitter final {
public:
	explicit MsgSplitter(uint32 protoTag)
	: _protoTag(protoTag) {
	}

	std::vector<QByteArray> split(
			const QByteArray &cipher,
			const QByteArray &plain) {
		if (cipher.isEmpty()) {
			return {};
		}
		if (_disabled) {
			return { cipher };
		}
		_cipherBuf.append(cipher);
		_plainBuf.append(plain);

		auto result = std::vector<QByteArray>();
		auto offset = 0;
		while (offset < _cipherBuf.size()) {
			const auto avail = _cipherBuf.size() - offset;
			const auto packetLen = nextPacketLen(offset, avail);
			if (packetLen <= 0) {
				if (packetLen < 0) {
					_disabled = true;
					result.push_back(_cipherBuf.mid(offset));
					offset = _cipherBuf.size();
				}
				break;
			}
			result.push_back(_cipherBuf.mid(offset, packetLen));
			offset += packetLen;
		}
		if (offset > 0) {
			_cipherBuf.remove(0, offset);
			_plainBuf.remove(0, offset);
		}
		return result;
	}

	std::vector<QByteArray> flush() {
		if (_cipherBuf.isEmpty()) {
			return {};
		}
		auto tail = std::exchange(_cipherBuf, QByteArray());
		_plainBuf.clear();
		return { std::move(tail) };
	}

private:
	int nextPacketLen(int offset, int avail) const {
		if (avail <= 0) {
			return 0;
		}
		if (_protoTag == kProtoTagAbridged) {
			const auto first = uchar(_plainBuf[offset]);
			if (first == 0x7F || first == 0xFF) {
				if (avail < 4) {
					return 0;
				}
				const auto len3 = uchar(_plainBuf[offset + 1])
					| (uchar(_plainBuf[offset + 2]) << 8)
					| (uchar(_plainBuf[offset + 3]) << 16);
				const auto payload = len3 * 4;
				if (payload <= 0) {
					return -1;
				}
				const auto packet = payload + 4;
				return (avail >= packet) ? packet : 0;
			}
			const auto payload = (first & 0x7F) * 4;
			if (payload <= 0) {
				return -1;
			}
			const auto packet = payload + 1;
			return (avail >= packet) ? packet : 0;
		}
		if (_protoTag == kProtoTagIntermediate || _protoTag == kProtoTagSecure) {
			if (avail < 4) {
				return 0;
			}
			const auto raw = qFromLittleEndian<uint32>(
				reinterpret_cast<const uchar*>(_plainBuf.constData() + offset));
			const auto payload = int(raw & 0x7FFFFFFFU);
			if (payload <= 0) {
				return -1;
			}
			const auto packet = payload + 4;
			return (avail >= packet) ? packet : 0;
		}
		return -1;
	}

	uint32 _protoTag = 0;
	QByteArray _cipherBuf;
	QByteArray _plainBuf;
	bool _disabled = false;
};

} // namespace Rabbit::TgWsProxy
