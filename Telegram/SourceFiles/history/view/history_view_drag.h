/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include <QtCore/QByteArray>
#include <QtGui/QImage>

class QMimeData;
class PhotoData;

namespace HistoryView {

struct PhotoDragData {
	QImage image;
	QByteArray bytes;
};

[[nodiscard]] PhotoDragData PreparePhotoDragData(not_null<PhotoData*> photo);

void FillDragMimeWithPhoto(not_null<QMimeData*> mime, PhotoDragData &&data);

} // namespace HistoryView
