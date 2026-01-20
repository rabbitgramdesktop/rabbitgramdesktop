/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "ui/peer/video_userpic_player.h"

#include "rabbit/settings/rabbit_settings.h"

#include "data/data_peer.h"
#include "data/data_photo.h"
#include "data/data_session.h"
#include "data/data_streaming.h"
#include "data/data_file_origin.h"
#include "media/streaming/media_streaming_instance.h"
#include "media/streaming/media_streaming_player.h"
#include "media/streaming/media_streaming_document.h"
#include "ui/image/image_prepare.h"
#include "ui/empty_userpic.h"
#include "ui/painter.h"
#include "styles/style_widgets.h"

namespace Ui {

VideoUserpicPlayer::VideoUserpicPlayer() = default;

VideoUserpicPlayer::~VideoUserpicPlayer() = default;

void VideoUserpicPlayer::setup(
		not_null<PeerData*> peer,
		not_null<PhotoData*> photo) {
	if (_streamedPhoto == photo && _streamed && _peer == peer) {
		return;
	}
	if (!createStreaming(peer, photo)) {
		photo->setVideoPlaybackFailed();
		return;
	}
	_streamedPhoto = photo;
	_peer = peer;
	checkStarted();
}

void VideoUserpicPlayer::clear() {
	_streamed = nullptr;
	_streamedPhoto = nullptr;
	_peer = nullptr;
}

QImage VideoUserpicPlayer::frame(QSize size, not_null<PeerData*> peer) {
	if (!_streamed
		|| !_streamed->player().ready()
		|| _streamed->player().videoSize().isEmpty()
		|| !_streamedPhoto) {
		return {};
	}
	auto request = ::Media::Streaming::FrameRequest();
	const auto ratio = style::DevicePixelRatio();
	request.outer = request.resize = size * ratio;

	_roundingCorners = Images::CornersMask(0);
	request.rounding = Images::CornersMaskRef(_roundingCorners);

	auto result = _streamed->frame(request);
	_streamed->markFrameShown();
	return result;
}

bool VideoUserpicPlayer::ready() const {
	return _streamed
		&& _streamed->player().ready()
		&& !_streamed->player().videoSize().isEmpty();
}

bool VideoUserpicPlayer::createStreaming(
		not_null<PeerData*> peer,
		not_null<PhotoData*> photo) {
	using namespace ::Media::Streaming;
	const auto origin = peer->isUser()
		? Data::FileOriginUserPhoto(peerToUser(peer->id), photo->id)
		: Data::FileOrigin(Data::FileOriginPeerPhoto(peer->id));
	_streamed = std::make_unique<Instance>(
		photo->owner().streaming().sharedDocument(photo, origin),
		nullptr);
	_streamed->lockPlayer();
	_streamed->player().updates(
	) | rpl::on_next_error([=](Update &&update) {
		handleUpdate(std::move(update));
	}, [=](Error &&error) {
		handleError(std::move(error));
	}, _streamed->lifetime());
	if (_streamed->ready()) {
		streamingReady(base::duplicate(_streamed->info()));
	}
	if (!_streamed->valid()) {
		clear();
		return false;
	}
	return true;
}

void VideoUserpicPlayer::checkStarted() {
	if (!_streamed) {
		return;
	} else if (_streamed->paused()) {
		_streamed->resume();
	}
	if (_streamed && !_streamed->active() && !_streamed->failed()) {
		const auto position = _streamedPhoto->videoStartPosition();
		auto options = ::Media::Streaming::PlaybackOptions();
		options.position = position;
		options.mode = ::Media::Streaming::Mode::Video;
		options.loop = true;
		_streamed->play(options);
	}
}

void VideoUserpicPlayer::handleUpdate(
		::Media::Streaming::Update &&update) {
	using namespace ::Media::Streaming;
	v::match(update.data, [&](Information &update) {
		streamingReady(std::move(update));
	}, [](PreloadedVideo) {
	}, [](UpdateVideo) {
	}, [](PreloadedAudio) {
	}, [](UpdateAudio) {
	}, [](WaitingForData) {
	}, [](SpeedEstimate) {
	}, [](MutedByOther) {
	}, [](Finished) {
	});
}

void VideoUserpicPlayer::handleError(::Media::Streaming::Error &&error) {
	_streamedPhoto->setVideoPlaybackFailed();
	clear();
}

void VideoUserpicPlayer::streamingReady(
		::Media::Streaming::Information &&info) {
}

} // namespace Ui
