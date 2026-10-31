/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/ui/settings/app_icon_box.h"

#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QRadialGradient>

#include "apiwrap.h"
#include "core/application.h"
#include "crl/crl_time.h"
#include "data/data_changes.h"
#include "data/data_peer.h"
#include "data/data_session.h"
#include "main/main_account.h"
#include "main/main_domain.h"
#include "main/main_session.h"
#include "rabbit/lang/rabbit_lang.h"
#include "rabbit/settings/rabbit_settings.h"
#include "rabbit/ui/rabbit_assets.h"
#include "ui/effects/animations.h"
#include "ui/empty_userpic.h"
#include "ui/painter.h"
#include "ui/userpic_view.h"
#include "ui/widgets/buttons.h"
#include "ui/wrap/vertical_layout.h"
#include "window/main_window.h"
#include "window/window_controller.h"
#include "window/window_session_controller.h"
#include "window/window_session_controller_link_info.h"

#include "styles/style_boxes.h"
#include "styles/style_layers.h"
#include "styles/style_premium.h"

namespace {

struct AppIconDescriptor {
	QString id;
	QString author;
	QColor accentColor;
};

[[nodiscard]] const std::vector<AppIconDescriptor> &GetIconVariants() {
	static const auto variants = std::vector<AppIconDescriptor>{
		{ RabbitAssets::DEFAULT_ICON, u"@xmdnusr"_q, QColor(42, 171, 238) },
		{ RabbitAssets::ANGEL_ICON, u"@MilesBlooper"_q, QColor(229, 169, 60) },
		{ RabbitAssets::BLACKOUT_ICON, u"@xmdnusr"_q, QColor(100, 108, 118) },
		{ RabbitAssets::COLOR_ICON, u"@MilesBlooper"_q, QColor(224, 64, 251) },
		{ RabbitAssets::IMPOSTOR_ICON, u"@MilesBlooper"_q, QColor(229, 57, 53) },
		{ RabbitAssets::MATRIX_ICON, u"@xmdnusr"_q, QColor(0, 230, 118) },
		{ RabbitAssets::MOON_ICON, u"@MilesBlooper"_q, QColor(92, 107, 192) },
		{ RabbitAssets::OLD_ICON, u"@xmdnusr"_q, QColor(30, 136, 229) },
		{ RabbitAssets::SEASONAL_ICON, u"@MilesBlooper"_q, QColor(240, 98, 146) },
		{ RabbitAssets::TWITCH_ICON, u"@MilesBlooper"_q, QColor(145, 70, 255) },
	};
	return variants;
}

[[nodiscard]] const AppIconDescriptor *FindDescriptor(const QString &id) {
	const auto &variants = GetIconVariants();
	for (const auto &v : variants) {
		if (v.id == id) {
			return &v;
		}
	}
	return variants.empty() ? nullptr : &variants.front();
}

[[nodiscard]] QString GetIconName(const QString &id) {
	return RabbitLang::Lang::Translate(u"rtg_settings_app_icon_"_q + id);
}

[[nodiscard]] QString GetIconAuthor(const AppIconDescriptor &descriptor) {
	if (!descriptor.author.isEmpty()) {
		return descriptor.author;
	}
	const auto authorKey = u"rtg_settings_app_icon_"_q + descriptor.id + u"_author"_q;
	const auto tr = RabbitLang::Lang::Translate(authorKey);
	if (tr != authorKey && !tr.isEmpty()) {
		return tr;
	}
	return QString();
}

[[nodiscard]] inline QColor InterpolateColor(const QColor &from, const QColor &to, float64 progress) {
	return QColor(
		std::clamp(int(from.red() + (to.red() - from.red()) * progress), 0, 255),
		std::clamp(int(from.green() + (to.green() - from.green()) * progress), 0, 255),
		std::clamp(int(from.blue() + (to.blue() - from.blue()) * progress), 0, 255),
		std::clamp(int(from.alpha() + (to.alpha() - from.alpha()) * progress), 0, 255));
}

[[nodiscard]] inline QRectF InterpolateRect(const QRectF &from, const QRectF &to, float64 progress) {
	return QRectF(
		from.x() + (to.x() - from.x()) * progress,
		from.y() + (to.y() - from.y()) * progress,
		from.width() + (to.width() - from.width()) * progress,
		from.height() + (to.height() - from.height()) * progress);
}

class AppIconHero final : public Ui::RpWidget {
public:
	AppIconHero(
		QWidget *parent,
		Window::SessionController *controller,
		Fn<void()> closeBox,
		rpl::producer<QString> selectedId)
		: RpWidget(parent)
		, _controller(controller)
		, _closeBox(std::move(closeBox)) {
		setMouseTracking(true);

		std::move(
			selectedId
		) | rpl::on_next([=](const QString &id) {
			onIdChanged(id);
		}, lifetime());

		widthValue() | rpl::on_next([=](int) {
			updateSize();
		}, lifetime());
	}

	~AppIconHero() {
		cancelResolveRequest();
	}

protected:
	void paintEvent(QPaintEvent *e) override {
		Painter p(this);
		PainterHighQualityEnabler hq(p);

		const auto w = width();
		const auto h = height();
		const auto cx = w / 2.0;
		const auto iconCenterY = 22.0 + 84.0 / 2.0;

		auto clipPath = QPainterPath();
		clipPath.addRoundedRect(QRectF(0, 0, w, h + st::boxRadius), st::boxRadius, st::boxRadius);
		p.setClipPath(clipPath);

		p.fillRect(rect(), st::boxBg);

		const auto radius = std::max(cx, static_cast<double>(h)) * 1.15;
		auto grad = QRadialGradient(QPointF(cx, iconCenterY), radius);

		auto centerColor = _accentColor;
		centerColor.setAlphaF(0.42f);
		auto midColor = _accentColor;
		midColor.setAlphaF(0.12f);
		auto edgeColor = st::boxBg->c;
		edgeColor.setAlphaF(0.0f);

		grad.setColorAt(0.0, centerColor);
		grad.setColorAt(0.48, midColor);
		grad.setColorAt(1.0, edgeColor);

		p.fillRect(rect(), grad);

		const auto iconSize = 84;
		const auto iconX = int((w - iconSize) / 2.0);
		const auto iconY = 22;
		const auto iconRect = QRect(iconX, iconY, iconSize, iconSize);

		if (_iconAnim.animating()) {
			const auto progress = _iconAnim.value(1.0);
			if (!_prevImage.isNull()) {
				p.save();
				p.setOpacity(1.0 - progress);
				const auto scale = 1.0 - 0.08 * progress;
				p.translate(iconRect.center());
				p.scale(scale, scale);
				p.translate(-iconRect.center());
				p.drawImage(iconRect, _prevImage);
				p.restore();
			}
			if (!_currImage.isNull()) {
				p.save();
				p.setOpacity(progress);
				const auto scale = 0.92 + 0.08 * progress;
				p.translate(iconRect.center());
				p.scale(scale, scale);
				p.translate(-iconRect.center());
				p.drawImage(iconRect, _currImage);
				p.restore();
			}
		} else if (!_currImage.isNull()) {
			p.drawImage(iconRect, _currImage);
		}

		p.setFont(st::boxTitleFont);
		p.setPen(st::windowFg);
		const auto nameY = iconY + iconSize + 12;
		const auto nameHeight = st::boxTitleFont->height;
		const auto nameRect = QRect(0, nameY, w, nameHeight);

		if (_textAnim.animating()) {
			p.save();
			p.setOpacity(_textAnim.value(1.0));
			p.drawText(nameRect, Qt::AlignCenter, _currName);
			p.restore();
		} else {
			p.drawText(nameRect, Qt::AlignCenter, _currName);
		}

		if (!_currAuthor.isEmpty()) {
			const auto rect = computeAuthorRect();
			const auto drawBadge = [&](float64 opacity) {
				p.save();
				if (opacity < 1.0) {
					p.setOpacity(opacity);
				}

				if (_authorHovered) {
					auto hoverBg = st::windowBgOver->c;
					hoverBg.setAlphaF(0.45f);
					p.setPen(Qt::NoPen);
					p.setBrush(hoverBg);
					p.drawRoundedRect(rect, rect.height() / 2.0, rect.height() / 2.0);
				}

				const auto userpicX = rect.x() + kBadgePaddingX;
				const auto userpicY = rect.y() + (rect.height() - kUserpicSize) / 2;
				if (_peer) {
					_peer->paintUserpic(p, _userpicView, userpicX, userpicY, kUserpicSize);
				} else {
					const auto cleanUsername = _currAuthor.startsWith('@')
						? _currAuthor.mid(1)
						: _currAuthor;
					const auto empty = Ui::EmptyUserpic(
						Ui::EmptyUserpic::UserpicColor(0),
						cleanUsername);
					const auto radius = int(kUserpicSize * RabbitSettings::userpicRoundness() / 100.0);
					empty.paintRounded(p, userpicX, userpicY, width(), kUserpicSize, radius);
				}

				p.setFont(st::normalFont);
				p.setPen(_authorHovered ? st::windowFg : st::windowSubTextFg);
				const auto textX = userpicX + kUserpicSize + kBadgeSpacing;
				const auto textY = rect.y() + (rect.height() - st::normalFont->height) / 2;
				const auto textWidth = rect.width() - (textX - rect.x()) - kBadgePaddingX;
				p.drawText(
					QRect(textX, textY, textWidth, st::normalFont->height),
					Qt::AlignLeft | Qt::AlignVCenter,
					_currAuthor);

				p.restore();
			};

			if (_textAnim.animating()) {
				drawBadge(_textAnim.value(1.0));
			} else {
				drawBadge(1.0);
			}
		}
	}

	void mouseMoveEvent(QMouseEvent *e) override {
		const auto rect = computeAuthorRect();
		const auto hovered = !rect.isEmpty() && rect.contains(e->pos());
		if (hovered != _authorHovered) {
			_authorHovered = hovered;
			setCursor(_authorHovered ? style::cur_pointer : style::cur_default);
			update();
		}
	}

	void leaveEventHook(QEvent *e) override {
		if (_authorHovered) {
			_authorHovered = false;
			unsetCursor();
			update();
		}
	}

	void mousePressEvent(QMouseEvent *e) override {
		if (e->button() == Qt::LeftButton) {
			const auto rect = computeAuthorRect();
			if (!rect.isEmpty() && rect.contains(e->pos())) {
				openProfile();
			}
		}
	}

private:
	static constexpr auto kUserpicSize = 20;
	static constexpr auto kBadgePaddingX = 8;
	static constexpr auto kBadgeSpacing = 6;
	static constexpr auto kBadgeHeight = 28;

	[[nodiscard]] QRect computeAuthorRect() const {
		if (_currAuthor.isEmpty()) {
			return QRect();
		}
		const auto font = st::normalFont;
		const auto textWidth = font->width(_currAuthor);
		const auto badgeWidth = kBadgePaddingX
			+ kUserpicSize
			+ kBadgeSpacing
			+ textWidth
			+ kBadgePaddingX;
		const auto iconSize = 84;
		const auto iconY = 22;
		const auto nameY = iconY + iconSize + 12;
		const auto nameHeight = st::boxTitleFont->height;
		const auto authorY = nameY + nameHeight + 6;
		const auto badgeX = (width() - badgeWidth) / 2;
		return QRect(badgeX, authorY, badgeWidth, kBadgeHeight);
	}

	void updateSize() {
		const auto nameHeight = st::boxTitleFont->height;
		const auto authorHeight = _currAuthor.isEmpty() ? 0 : (kBadgeHeight + 6);
		const auto totalHeight = 22 + 84 + 12 + nameHeight + authorHeight + 20;
		resize(width(), totalHeight);
	}

	void openProfile() {
		const auto cleanUsername = _currAuthor.startsWith('@')
			? _currAuthor.mid(1)
			: _currAuthor;
		if (cleanUsername.isEmpty()) {
			return;
		}

		const auto controller = _controller
			? _controller
			: (Core::App().activeWindow() ? Core::App().activeWindow()->sessionController() : nullptr);
		if (!controller) {
			return;
		}

		SetPendingReopenAppIconId(_currentId);

		const auto peer = _peer;
		const auto close = _closeBox;
		if (close) {
			close();
		}

		if (peer) {
			controller->showPeerInfo(peer);
		} else {
			controller->showPeerByLink(Window::PeerByLinkInfo{
				.usernameOrId = cleanUsername,
				.resolveType = Window::ResolveType::Profile,
			});
		}
	}

	void cancelResolveRequest() {
		if (_resolveRequestId) {
			const auto session = resolveSession();
			if (session) {
				session->api().request(base::take(_resolveRequestId)).cancel();
			} else {
				_resolveRequestId = 0;
			}
		}
	}

	[[nodiscard]] Main::Session *resolveSession() const {
		if (_controller) {
			return &_controller->session();
		} else if (const auto win = Core::App().activeWindow()) {
			if (const auto controller = win->sessionController()) {
				return &controller->session();
			}
		}
		return Core::App().domain().active().maybeSession();
	}

	void resolveAuthorPeer(const QString &author) {
		cancelResolveRequest();
		_peer = nullptr;
		_peerLifetime.destroy();
		_userpicView = Ui::PeerUserpicView();

		const auto cleanUsername = author.startsWith('@') ? author.mid(1) : author;
		if (cleanUsername.isEmpty()) {
			update();
			return;
		}

		const auto session = resolveSession();
		if (!session) {
			update();
			return;
		}

		if (const auto peer = session->data().peerByUsername(cleanUsername)) {
			setPeer(peer);
		} else {
			_resolveRequestId = session->api().request(MTPcontacts_ResolveUsername(
				MTP_flags(0),
				MTP_string(cleanUsername),
				MTP_string()
			)).done(crl::guard(this, [=](const MTPcontacts_ResolvedPeer &result) {
				_resolveRequestId = 0;
				const auto session = resolveSession();
				if (!session) {
					return;
				}
				session->data().processUsers(result.data().vusers());
				session->data().processChats(result.data().vchats());
				const auto resolved = session->data().peerLoaded(
					peerFromMTP(result.data().vpeer()));
				if (resolved) {
					const auto currentAuthorClean = _currAuthor.startsWith('@')
						? _currAuthor.mid(1)
						: _currAuthor;
					if (currentAuthorClean.compare(cleanUsername, Qt::CaseInsensitive) == 0) {
						setPeer(resolved);
					}
				}
			})).fail(crl::guard(this, [=] {
				_resolveRequestId = 0;
			})).send();
		}
	}

	void setPeer(not_null<PeerData*> peer) {
		_peer = peer;
		_userpicView = Ui::PeerUserpicView();
		_peer->loadUserpic();
		_peerLifetime.destroy();
		_peer->session().changes().peerFlagsValue(
			_peer,
			Data::PeerUpdate::Flag::Photo
		) | rpl::on_next([=] {
			_userpicView = Ui::PeerUserpicView();
			update();
		}, _peerLifetime);
		update();
	}

	void onIdChanged(const QString &id) {
		const auto desc = FindDescriptor(id);
		const auto newColor = desc ? desc->accentColor : QColor(42, 171, 238);
		const auto newName = GetIconName(id);
		const auto newAuthor = desc ? GetIconAuthor(*desc) : QString();
		const auto newImage = RabbitAssets::loadPreview(id);

		if (_currentId.isEmpty()) {
			_currentId = id;
			_accentColor = newColor;
			_currName = newName;
			_currAuthor = newAuthor;
			_currImage = newImage;
			resolveAuthorPeer(_currAuthor);
			updateSize();
			update();
			return;
		}

		_currentId = id;
		_fromColor = _accentColor;
		_toColor = newColor;
		_colorAnim.start(
			[=] {
				_accentColor = InterpolateColor(_fromColor, _toColor, _colorAnim.value(1.0));
				update();
			},
			0.0,
			1.0,
			300,
			anim::easeOutCubic);

		_prevImage = _currImage;
		_currImage = newImage;
		_iconAnim.start([=] { update(); }, 0.0, 1.0, 250, anim::easeOutCubic);

		_prevName = _currName;
		_currName = newName;
		_prevAuthor = _currAuthor;
		_currAuthor = newAuthor;
		resolveAuthorPeer(_currAuthor);
		updateSize();
		_textAnim.start([=] { update(); }, 0.0, 1.0, 200, anim::easeOutCubic);
	}

	Window::SessionController *_controller = nullptr;
	Fn<void()> _closeBox;

	QString _currentId;
	QColor _accentColor;
	QColor _fromColor;
	QColor _toColor;
	Ui::Animations::Simple _colorAnim;

	QImage _prevImage;
	QImage _currImage;
	Ui::Animations::Simple _iconAnim;

	QString _prevName;
	QString _currName;
	QString _prevAuthor;
	QString _currAuthor;
	Ui::Animations::Simple _textAnim;

	PeerData *_peer = nullptr;
	Ui::PeerUserpicView _userpicView;
	rpl::lifetime _peerLifetime;
	mtpRequestId _resolveRequestId = 0;
	bool _authorHovered = false;

};

class AppIconGrid final : public Ui::RpWidget {
public:
	AppIconGrid(
		QWidget *parent,
		rpl::producer<QString> selectedId,
		Fn<void(QString)> onSelect)
		: RpWidget(parent)
		, _onSelect(std::move(onSelect)) {
		setMouseTracking(true);
		setCursor(style::cur_pointer);

		std::move(
			selectedId
		) | rpl::on_next([=](const QString &id) {
			onIdChanged(id);
		}, lifetime());

		widthValue() | rpl::on_next([=](int) {
			updateSize();
		}, lifetime());
	}

protected:
	void paintEvent(QPaintEvent *e) override {
		Painter p(this);
		PainterHighQualityEnabler hq(p);

		const auto &variants = GetIconVariants();
		const auto count = static_cast<int>(variants.size());

		const auto drawBackdrop = [&](const QRectF &r, float64 scale, float64 alpha) {
			if (alpha <= 0.001 || scale <= 0.001) {
				return;
			}
			p.save();
			p.setOpacity(alpha);
			const auto center = r.center();
			p.translate(center);
			p.scale(scale, scale);
			p.translate(-center);
			p.setPen(Qt::NoPen);
			p.setBrush(st::windowBgOver);
			p.drawRoundedRect(r, 12.0, 12.0);
			p.restore();
		};

		if (_selectionAnim.animating()) {
			const auto progress = _selectionAnim.value(1.0);
			if (_prevSelectedIndex >= 0) {
				drawBackdrop(cellRect(_prevSelectedIndex), 1.0, 1.0 - progress);
			}
			if (_selectedIndex >= 0) {
				const auto bounce = 0.94 + 0.06 * anim::easeOutBack(1., progress);
				drawBackdrop(cellRect(_selectedIndex), bounce, progress);
			}
		} else if (_selectedIndex >= 0) {
			drawBackdrop(cellRect(_selectedIndex), 1.0, 1.0);
		}

		if (_hoverIndex >= 0 && _hoverIndex != _selectedIndex && _hoverIndex < count) {
			p.setPen(Qt::NoPen);
			auto hoverBg = st::windowBgOver->c;
			hoverBg.setAlphaF(0.45f);
			p.setBrush(hoverBg);
			p.drawRoundedRect(cellRect(_hoverIndex), 12.0, 12.0);
		}

		for (int i = 0; i < count; ++i) {
			const auto cRect = cellRect(i);
			const auto &desc = variants[i];
			const auto iconX = int(cRect.x() + (cRect.width() - kIconSize) / 2.0);
			const auto iconY = int(cRect.y() + kCellPaddingTop);
			const auto iconRect = QRect(iconX, iconY, kIconSize, kIconSize);

			const auto iconImg = RabbitAssets::loadPreview(desc.id);
			p.drawImage(iconRect, iconImg);

			p.setFont(st::normalFont);
			p.setPen(st::windowFg);
			const auto textY = iconY + kIconSize + kCellSpacing;
			const auto textRect = QRect(int(cRect.x()), textY, int(cRect.width()), st::normalFont->height);
			p.drawText(textRect, Qt::AlignCenter, GetIconName(desc.id));

			const auto badgeRadius = 10.0;
			const auto badgeCenter = QPointF(
				iconX + kIconSize - badgeRadius,
				iconY + kIconSize - badgeRadius);
			const auto backdropColor = st::windowBgOver->c;

			auto drawCheck = [&](float64 scale) {
				if (scale <= 0.001) {
					return;
				}
				p.save();
				p.translate(badgeCenter);
				p.scale(scale, scale);
				p.translate(-badgeCenter);

				p.setPen(QPen(backdropColor, 2.5));
				p.setBrush(desc.accentColor);
				p.drawEllipse(badgeCenter, badgeRadius, badgeRadius);

				auto checkPath = QPainterPath();
				checkPath.moveTo(badgeCenter.x() - 4.2, badgeCenter.y() - 0.4);
				checkPath.lineTo(badgeCenter.x() - 1.2, badgeCenter.y() + 3.0);
				checkPath.lineTo(badgeCenter.x() + 4.2, badgeCenter.y() - 3.2);
				p.strokePath(checkPath, QPen(Qt::white, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

				p.restore();
			};

			if (i == _selectedIndex) {
				const auto scale = _selectionAnim.animating()
					? anim::easeOutBack(1., _selectionAnim.value(1.0))
					: 1.0;
				drawCheck(scale);
			} else if (i == _prevSelectedIndex && _selectionAnim.animating()) {
				const auto scale = 1.0 - _selectionAnim.value(1.0);
				drawCheck(scale);
			}
		}
	}

	void mouseMoveEvent(QMouseEvent *e) override {
		const auto newHover = indexByPos(e->pos());
		if (newHover != _hoverIndex) {
			_hoverIndex = newHover;
			update();
		}
	}

	void leaveEventHook(QEvent *e) override {
		if (_hoverIndex != -1) {
			_hoverIndex = -1;
			update();
		}
	}

	void mousePressEvent(QMouseEvent *e) override {
		const auto index = indexByPos(e->pos());
		const auto &variants = GetIconVariants();
		if (index >= 0 && index < static_cast<int>(variants.size())) {
			const auto &id = variants[index].id;
			if (id != _selectedId) {
				_prevSelectedIndex = _selectedIndex;
				_selectedIndex = index;
				_selectedId = id;
				_selectionAnim.start([=] { update(); }, 0.0, 1.0, 240, anim::easeOutCubic);
				if (_onSelect) {
					_onSelect(id);
				}
			}
		}
	}

private:
	static constexpr auto kColumns = 4;
	static constexpr auto kIconSize = 60;
	static constexpr auto kCellPaddingTop = 8;
	static constexpr auto kCellSpacing = 6;
	static constexpr auto kCellPaddingBottom = 8;
	static constexpr auto kRowGap = 8;
	static constexpr auto kSideMargin = 12;

	[[nodiscard]] int cellHeight() const {
		return kCellPaddingTop + kIconSize + kCellSpacing + st::normalFont->height + kCellPaddingBottom;
	}

	[[nodiscard]] int rowHeight() const {
		return cellHeight() + kRowGap;
	}

	[[nodiscard]] QRectF cellRect(int index) const {
		if (index < 0 || index >= static_cast<int>(GetIconVariants().size())) {
			return QRectF();
		}
		const auto col = index % kColumns;
		const auto row = index / kColumns;
		const auto totalW = width() - 2 * kSideMargin;
		const auto colW = float64(totalW) / kColumns;
		const auto x = kSideMargin + col * colW;
		const auto y = row * rowHeight();
		return QRectF(x + 2, y + 2, colW - 4, cellHeight() - 4);
	}

	[[nodiscard]] int indexByPos(QPoint pos) const {
		const auto &variants = GetIconVariants();
		const auto count = static_cast<int>(variants.size());
		for (int i = 0; i < count; ++i) {
			if (cellRect(i).contains(pos)) {
				return i;
			}
		}
		return -1;
	}

	void updateSize() {
		const auto count = static_cast<int>(GetIconVariants().size());
		const auto rows = (count + kColumns - 1) / kColumns;
		const auto totalH = rows * rowHeight() + 8;
		resize(width(), totalH);
	}

	void onIdChanged(const QString &id) {
		if (_selectedId == id) {
			return;
		}
		_selectedId = id;
		const auto &variants = GetIconVariants();
		auto newIdx = -1;
		for (int i = 0; i < static_cast<int>(variants.size()); ++i) {
			if (variants[i].id == id) {
				newIdx = i;
				break;
			}
		}
		if (_selectedIndex == -1) {
			_selectedIndex = newIdx;
			update();
		} else if (_selectedIndex != newIdx) {
			_prevSelectedIndex = _selectedIndex;
			_selectedIndex = newIdx;
			_selectionAnim.start([=] { update(); }, 0.0, 1.0, 240, anim::easeOutCubic);
		}
	}

	Fn<void(QString)> _onSelect;
	QString _selectedId;
	int _selectedIndex = -1;
	int _prevSelectedIndex = -1;
	int _hoverIndex = -1;
	Ui::Animations::Simple _selectionAnim;

};

QString gPendingReopenAppIconId;
crl::time gPendingReopenTime = 0;

} // namespace

void SetPendingReopenAppIconId(const QString &id) {
	gPendingReopenAppIconId = id;
	gPendingReopenTime = crl::now();
}

QString TakePendingReopenAppIconId() {
	if (gPendingReopenTime == 0 || crl::now() - gPendingReopenTime > 10000) {
		gPendingReopenAppIconId = QString();
		gPendingReopenTime = 0;
		return QString();
	}
	gPendingReopenTime = 0;
	return std::exchange(gPendingReopenAppIconId, QString());
}

void AppIconBox(
		not_null<Ui::GenericBox*> box,
		Window::SessionController *controller,
		const QString &selectedId) {
	const auto currentAppIcon = RabbitSettings::appIcon().isEmpty()
		? RabbitAssets::DEFAULT_ICON
		: RabbitSettings::appIcon();
	const auto initialId = !selectedId.isEmpty()
		? selectedId
		: currentAppIcon;

	const auto selectedIdVar = box->lifetime().make_state<rpl::variable<QString>>(initialId);

	box->setWidth(st::boxWideWidth);
	box->setNoContentMargin(true);
	box->setCustomCornersFilling(RectPart::FullTop);
	box->addTopButton(st::boxTitleClose, [=] { box->closeBox(); });

	const auto content = box->verticalLayout();
	content->add(object_ptr<AppIconHero>(
		content,
		controller,
		[=] { box->closeBox(); },
		selectedIdVar->value()));
	content->add(object_ptr<Ui::RpWidget>(content), { 0, 8, 0, 8 });
	content->add(object_ptr<AppIconGrid>(
		content,
		selectedIdVar->value(),
		[=](QString id) {
			*selectedIdVar = id;
		}));
	content->add(object_ptr<Ui::RpWidget>(content), { 0, 0, 0, 16 });

	const auto &st = st::premiumPreviewDoubledLimitsBox;
	box->setStyle(st);

	auto button = object_ptr<Ui::RoundButton>(
		box,
		rpl::single(QString()),
		st::defaultActiveButton);

	button->resizeToWidth(box->width()
		- st.buttonPadding.left()
		- st.buttonPadding.right());

	selectedIdVar->value() | rpl::on_next([=, raw = button.data()](const QString &id) {
		const auto isChanged = (id != currentAppIcon);
		raw->setText(rktr(isChanged
			? u"rtg_settings_app_icon_choose"_q
			: u"rtg_settings_app_icon_keep"_q));
		if (isChanged) {
			raw->setBrushOverride(std::nullopt);
			raw->setTextFgOverride(std::nullopt);
		} else {
			raw->setBrushOverride(st::windowBgOver->b);
			raw->setTextFgOverride(st::windowFg->c);
		}
	}, button->lifetime());

	button->setClickedCallback([=] {
		if (selectedIdVar->current() != currentAppIcon) {
			RabbitSettings::setAppIcon(selectedIdVar->current());
			RabbitAssets::applyAppIcon();
		}
		box->closeBox();
	});

	box->addButton(std::move(button));
}
