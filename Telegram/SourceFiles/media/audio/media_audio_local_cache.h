/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace Media::Audio {

struct LocalSound {
    DocumentId id = 0;
    QByteArray wav;

    explicit operator bool() const {
        return !wav.isEmpty();
    }
};

class LocalCache final {
public:
    [[nodiscard]] LocalSound sound(
        DocumentId id,
        Fn<QByteArray()> resolveOriginalBytes,
        Fn<QByteArray()> fallbackOriginalBytes);

private:
    base::flat_map<DocumentId, QByteArray> _cache;

};

class LocalDiskCache final {
public:
    explicit LocalDiskCache(const QString &folder);

    [[nodiscard]] QString name(const LocalSound &sound);
    [[nodiscard]] QString path(const LocalSound &sound);

private:
    const QString _base;
	base::flat_map<DocumentId, QString> _paths;

};

} // namespace Media::Audio
