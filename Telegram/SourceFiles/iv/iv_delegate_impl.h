/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "iv/iv_delegate.h"

namespace Iv {

class DelegateImpl final : public Delegate {
public:
	DelegateImpl() = default;

	[[nodiscard]] QRect ivGeometry(not_null<Ui::RpWindow*> window) const override;
	void ivSaveGeometry(not_null<Ui::RpWindow*> window) override;

	[[nodiscard]] int ivZoom() const override;
	[[nodiscard]] rpl::producer<int> ivZoomValue() const override;
	void ivSetZoom(int value) override;

};

} // namespace Iv
