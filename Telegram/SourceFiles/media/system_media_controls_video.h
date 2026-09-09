/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace Media {

class SystemMediaControlsVideoDelegate {
public:
	virtual void smtcPlay() = 0;
	virtual void smtcPause() = 0;
	virtual void smtcPlayPause() = 0;
	virtual void smtcStop() = 0;
	virtual void smtcNext() = 0;
	virtual void smtcPrevious() = 0;
	virtual void smtcSeek(crl::time position) = 0;

protected:
	~SystemMediaControlsVideoDelegate() = default;
};

class SystemMediaControlsVideoSink {
public:
	struct VideoState {
		QString title;
		QString artist;
		crl::time position = 0;
		crl::time duration = 0;
		bool playing = false;
		bool nextAvailable = false;
		bool previousAvailable = false;
	};

	virtual void videoStart(
		not_null<SystemMediaControlsVideoDelegate*> delegate,
		VideoState state) = 0;
	virtual void videoUpdate(VideoState state) = 0;
	virtual void videoSetThumbnail(const QImage &thumbnail) = 0;
	virtual void videoFinish(
		not_null<SystemMediaControlsVideoDelegate*> delegate) = 0;

protected:
	~SystemMediaControlsVideoSink() = default;
};

} // namespace Media
