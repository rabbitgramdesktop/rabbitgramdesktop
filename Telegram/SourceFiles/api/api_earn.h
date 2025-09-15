/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

class ChannelData;

namespace Ui {
class RippleButton;
class Show;
} // namespace Ui

namespace Api {

void RestrictSponsored(
	not_null<ChannelData*> channel,
	bool restricted,
	Fn<void(QString)> failed);

struct RewardReceiver final {
	PeerData *currencyReceiver = nullptr;
	PeerData *creditsReceiver = nullptr;
	Fn<uint64()> creditsAmount;
};

void HandleWithdrawalButton(
	RewardReceiver receiver,
	not_null<Ui::RippleButton*> button,
	std::shared_ptr<Ui::Show> show);

} // namespace Api
