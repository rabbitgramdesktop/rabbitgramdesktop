/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/settings_menu/sections/rabbit_chats.h"

#include "core/application.h"
#include "data/data_session.h"
#include "main/main_session.h"
#include "rabbit/lang/rabbit_lang.h"
#include "rabbit/settings/quick_action_ui.h"
#include "rabbit/settings/rabbit_settings.h"
#include "rabbit/settings_menu/boxes/quick_action_box.h"
#include "rabbit/settings_menu/rabbit_context_menu.h"
#include "rabbit/settings_menu/rabbit_settings_menu.h"
#include "rabbit/ui/settings/previews.h"
#include "settings/settings_builder.h"
#include "settings/settings_common.h"
#include "ui/cached_round_corners.h"
#include "ui/chat/chat_style.h"
#include "ui/boxes/confirm_box.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/checkbox.h"
#include "ui/widgets/continuous_sliders.h"
#include "ui/widgets/labels.h"
#include "ui/wrap/slide_wrap.h"
#include "ui/wrap/vertical_layout.h"
#include "ui/painter.h"
#include "window/window_session_controller.h"
#include "lang_auto.h"
#include "mainwindow.h"
#include "lang/lang_text_entity.h"

#include "styles/style_layers.h"
#include "styles/style_menu_icons.h"
#include "styles/style_settings.h"
#include "styles/style_info.h"
#include "styles/style_edit_peer_members.h"
#include "ui/text/text_utilities.h"

namespace Settings {
namespace {

using namespace Builder;

[[nodiscard]] RabbitSettings::QuickAction QuickActionFrom(int value) {
	return static_cast<RabbitSettings::QuickAction>(value);
}

void AddToggle(
	SectionBuilder &builder,
	QString id,
	rpl::producer<QString> title,
	const style::icon *icon,
	const Fn<bool()> &getter,
	const Fn<void(bool)> &setter,
	QStringList keywords = {}) {
	const auto st = icon
		? &st::settingsButton
		: &st::settingsButtonNoIcon;
	const auto controlId = id;
	if (const auto button = builder.addButton({
		.id = std::move(id),
		.title = std::move(title),
		.st = st,
		.icon = { icon },
		.toggled = rpl::single(getter()),
		.keywords = std::move(keywords),
	})) {
		button->toggledValue()
			| rpl::filter([=](bool enabled) { return enabled != getter(); })
			| rpl::on_next([=](bool enabled) { setter(enabled); },
				button->lifetime());
		if (const auto controller = builder.controller()) {
			RtgMenu::AttachSettingsContextMenu(
				button, controlId, controller);
		}
	}
}

void BuildTopGeneral(SectionBuilder &builder) {
	builder.add([](const WidgetContext &ctx) {
		auto wrap = object_ptr<Ui::VerticalLayout>(ctx.container);

		const auto chatPreview = wrap->add(
			object_ptr<ChatPreview>(wrap),
			st::defaultSubsectionTitlePadding);

		const auto stickerSizeLabel = wrap->add(
			object_ptr<Ui::LabelSimple>(wrap, st::settingsAudioVolumeLabel),
			st::settingsAudioVolumeLabelPadding);
		const auto stickerSizeSlider = wrap->add(
			object_ptr<Ui::MediaSlider>(wrap, st::settingsAudioVolumeSlider),
			st::settingsAudioVolumeSliderPadding);
		const auto updateStickerSizeLabel = [=](int value) {
			const auto pixels = QString::number(value);
			stickerSizeLabel->setText(ktr(
				u"rtg_chats_sticker_size"_q,
				{ u"pixels"_q, pixels }));
		};
		const auto updateStickerSize = [=](int value) {
			updateStickerSizeLabel(value);
			RabbitSettings::setStickerSize(value);
			chatPreview->repaint();
		};
		stickerSizeSlider->resize(st::settingsAudioVolumeSlider.seekSize);
		stickerSizeSlider->setPseudoDiscrete(
			193,
			[](int val) { return val + 64; },
			RabbitSettings::stickerSize(),
			updateStickerSize);
		updateStickerSizeLabel(RabbitSettings::stickerSize());

		const auto messageRoundnessLabel = wrap->add(
			object_ptr<Ui::LabelSimple>(wrap, st::settingsAudioVolumeLabel),
			st::settingsAudioVolumeLabelPadding);
		const auto messageRoundnessSlider = wrap->add(
			object_ptr<Ui::MediaSlider>(wrap, st::settingsAudioVolumeSlider),
			st::settingsAudioVolumeSliderPadding);
		const auto updateMessageRoundnessLabel = [=](int value) {
			const auto radius = QString::number(value);
			messageRoundnessLabel->setText(ktr(
				u"rtg_chats_message_rounding"_q,
				{ u"radius"_q, radius }));
		};
		const auto updateMessageRoundness = [=](int value) {
			updateMessageRoundnessLabel(value);
			RabbitSettings::setMessageRoundness(value);
			Ui::ClearCachedCornerMasks();
			ctx.controller->chatStyle()->clearCornerCaches();
			chatPreview->repaint();
		};
		messageRoundnessSlider->resize(
			st::settingsAudioVolumeSlider.seekSize);
		messageRoundnessSlider->setPseudoDiscrete(
			51,
			[](int val) { return val; },
			RabbitSettings::messageRoundness(),
			updateMessageRoundness);
		updateMessageRoundnessLabel(RabbitSettings::messageRoundness());

		return SectionBuilder::WidgetToAdd{ .widget = std::move(wrap) };
	}, [] {
		return SearchEntry{
			.id = u"rabbit/chats/sliders"_q,
			.title = ktr(u"rtg_settings_chats"_q),
		};
	});

	builder.add(nullptr, [] {
		return SearchEntry{
			.id = u"rabbit/chats/sticker_size"_q,
			.title = ktr(u"rtg_chats_sticker_size"_q),
			.keywords = { u"stickers"_q, u"size"_q },
		};
	});
	builder.add(nullptr, [] {
		return SearchEntry{
			.id = u"rabbit/chats/message_rounding"_q,
			.title = ktr(u"rtg_chats_message_rounding"_q),
			.keywords = { u"messages"_q, u"rounding"_q, u"radius"_q },
		};
	});

	AddToggle(
		builder,
		u"rabbit/chats/show_actions_time"_q,
		rktr(u"rtg_show_actions_time"_q),
		&st::menuIconReschedule,
		[] { return RabbitSettings::showActionsTime(); },
		[](bool value) { RabbitSettings::setShowActionsTime(value); },
		{ u"actions"_q, u"time"_q });
}

void BuildChats(SectionBuilder &builder) {
	builder.addSubsectionTitle({
		.id = u"rabbit/chats/settings"_q,
		.title = rktr(u"rtg_settings_chats"_q),
		.keywords = { u"chats"_q, u"messages"_q },
	});

	AddToggle(
		builder,
		u"rabbit/chats/comma_after_mention"_q,
		rktr(u"rtg_comma_after_mention"_q),
		nullptr,
		[] { return RabbitSettings::commaAfterMention(); },
		[](bool value) { RabbitSettings::setCommaAfterMention(value); },
		{ u"mention"_q, u"comma"_q });
}

void BuildMessages(SectionBuilder &builder) {
	builder.addSubsectionTitle({
		.id = u"rabbit/chats/messages"_q,
		.title = rktr(u"rtg_chats_messages"_q),
		.keywords = { u"messages"_q },
	});

	AddToggle(
		builder,
		u"rabbit/chats/hide_bubble_tails"_q,
		rktr(u"rtg_hide_bubble_tails"_q),
		nullptr,
		[] { return RabbitSettings::hideBubbleTails(); },
		[](bool value) { RabbitSettings::setHideBubbleTails(value); },
		{ u"bubble"_q, u"tails"_q });
	AddToggle(
		builder,
		u"rabbit/chats/replace_edited_label"_q,
		rktr(u"rtg_replace_edited_label"_q),
		nullptr,
		[] { return RabbitSettings::replaceEditedLabel(); },
		[](bool value) { RabbitSettings::setReplaceEditedLabel(value); },
		{ u"edited"_q, u"label"_q, u"icon"_q });
	AddToggle(
		builder,
		u"rabbit/chats/show_online_indicator"_q,
		rktr(u"rtg_show_online_indicator"_q),
		nullptr,
		[] { return RabbitSettings::showOnlineIndicator(); },
		[](bool value) { RabbitSettings::setShowOnlineIndicator(value); },
		{ u"online"_q, u"indicator"_q });
}

void BuildStickerShape(SectionBuilder &builder) {
	builder.addSubsectionTitle({
		.id = u"rabbit/chats/sticker_shape"_q,
		.title = rktr(u"rtg_chats_sticker_shape"_q),
		.keywords = { u"sticker"_q, u"shape"_q },
	});

	builder.add([](const WidgetContext &ctx) {
		return SectionBuilder::WidgetToAdd{
			.widget = object_ptr<StickerShapePicker>(ctx.container),
			.margin = st::defaultSubsectionTitlePadding,
		};
	});

	builder.add(nullptr, [] {
		return SearchEntry{
			.id = u"rabbit/chats/sticker_shape_picker"_q,
			.title = ktr(u"rtg_chats_sticker_shape"_q),
			.keywords = { u"sticker"_q, u"shape"_q },
		};
	});
}

void BuildQuickActions(
	SectionBuilder &builder,
	Window::SessionController *controller) {
	builder.addSubsectionTitle({
		.id = u"rabbit/chats/quick_actions"_q,
		.title = rktr(u"rtg_chats_quick_actions"_q),
		.keywords = { u"quick"_q, u"actions"_q, u"swipe"_q },
	});

	const auto incomingLabel = rpl::single(
		RabbitSettings::incomingQuickAction()
	) | rpl::then(
		RabbitSettings::JsonSettings::Events(u"incoming_quick_action"_q)
		| rpl::map([] { return RabbitSettings::incomingQuickAction(); })
	) | rpl::map([](int val) {
		return RabbitSettings::QuickActionString(QuickActionFrom(val));
	}) | rpl::flatten_latest();

	const auto outgoingLabel = rpl::single(
		RabbitSettings::outgoingQuickAction()
	) | rpl::then(
		RabbitSettings::JsonSettings::Events(u"outgoing_quick_action"_q)
		| rpl::map([] { return RabbitSettings::outgoingQuickAction(); })
	) | rpl::map([](int val) {
		return RabbitSettings::QuickActionString(QuickActionFrom(val));
	}) | rpl::flatten_latest();

	builder.add([](const WidgetContext &ctx) {
		return SectionBuilder::WidgetToAdd{
			.widget = object_ptr<QuickActionsPreview>(ctx.container),
			.margin = st::defaultSubsectionTitlePadding,
		};
	});

	if (const auto button = builder.addButton({
		.id = u"rabbit/chats/quick_actions/incoming"_q,
		.title = rktr(u"rtg_incoming_quick_actions"_q),
		.st = &st::settingsButtonNoIcon,
		.label = std::move(incomingLabel),
		.onClick = controller
			? [=] { controller->show(Box(IncomingQuickActionBox)); }
			: Fn<void()>(),
		.keywords = { u"incoming"_q, u"actions"_q },
	})) {
		if (controller) {
			RtgMenu::AttachSettingsContextMenu(
				button,
				u"rabbit/chats/quick_actions/incoming"_q,
				controller);
		}
	}

	if (const auto button = builder.addButton({
		.id = u"rabbit/chats/quick_actions/outgoing"_q,
		.title = rktr(u"rtg_outgoing_quick_actions"_q),
		.st = &st::settingsButtonNoIcon,
		.label = std::move(outgoingLabel),
		.onClick = controller
			? [=] { controller->show(Box(OutgoingQuickActionBox)); }
			: Fn<void()>(),
		.keywords = { u"outgoing"_q, u"actions"_q },
	})) {
		if (controller) {
			RtgMenu::AttachSettingsContextMenu(
				button,
				u"rabbit/chats/quick_actions/outgoing"_q,
				controller);
		}
	}

	if (const auto button = builder.addButton({
		.id = u"rabbit/chats/swipe_actions/manage"_q,
		.title = rktr(u"rtg_chats_swipe_actions"_q),
		.st = &st::settingsButtonNoIcon,
		.onClick = controller
			? [=] {
				auto box = Box([=](not_null<Ui::GenericBox*> box) {
					SwipeActionsBox(box);
				});
				controller->show(std::move(box));
			}
			: Fn<void()>(),
		.keywords = { u"swipe"_q, u"actions"_q },
	})) {
		if (controller) {
			RtgMenu::AttachSettingsContextMenu(
				button,
				u"rabbit/chats/swipe_actions/manage"_q,
				controller);
		}

		const auto iconsWidget = Ui::CreateChild<Ui::RpWidget>(button);
		iconsWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
		iconsWidget->show();

		const auto updateIconsGeometry = [=] {
			const auto list = RabbitSettings::swipeActionsList();
			auto totalWidth = 0;
			auto maxHeight = 0;
			for (const auto act : list) {
				if (const auto icon = RabbitSettings::QuickActionIcon(act).icon) {
					totalWidth += icon->width() + 4;
					maxHeight = std::max(maxHeight, icon->height());
				}
			}
			if (totalWidth > 0) {
				totalWidth -= 4;
			}
			const auto rightMargin = st::settingsButton.padding.right();
			const auto x = button->width() - totalWidth - rightMargin;
			const auto y = (button->height() - maxHeight) / 2;
			iconsWidget->setGeometry(x, y, totalWidth, maxHeight);
			iconsWidget->update();
		};

		button->sizeValue(
		) | rpl::on_next([=](QSize size) {
			updateIconsGeometry();
		}, button->lifetime());

		RabbitSettings::JsonSettings::Events(
			u"swipe_actions_list"_q
		) | rpl::on_next([=] {
			updateIconsGeometry();
		}, button->lifetime());

		iconsWidget->paintRequest(
		) | rpl::on_next([=] {
			auto p = QPainter(iconsWidget);
			const auto list = RabbitSettings::swipeActionsList();
			auto x = 0;
			for (const auto act : list) {
				if (const auto icon = RabbitSettings::QuickActionIcon(act).icon) {
					icon->paint(p, x, (iconsWidget->height() - icon->height()) / 2, iconsWidget->width());
					x += icon->width() + 4;
				}
			}
		}, iconsWidget->lifetime());
	}
}

object_ptr<Ui::RpWidget> CreateHideReactionsControl(
		not_null<QWidget*> parent,
		not_null<Window::SessionController*> controller) {
	auto container = object_ptr<Ui::VerticalLayout>(parent);
	const auto containerRaw = container.data();

	const auto &st = st::rightsButton;

	// Main button
	const auto button = containerRaw->add(object_ptr<Ui::SettingsButton>(
		containerRaw,
		nullptr,
		st));

	// Toggle button on the right
	const auto toggleButton = Ui::CreateChild<Ui::SettingsButton>(
		containerRaw,
		nullptr,
		st);

	struct State final {
		State(const style::Toggle &st, Fn<void()> c)
		: checkView(st, false, c) {
		}
		Ui::ToggleView checkView;
		Ui::Animations::Simple animation;
	};
	const auto state = button->lifetime().make_state<State>(
		st.toggle,
		[=] { toggleButton->update(); });

	const auto checkView = &state->checkView;

	// Separator
	const auto separator = Ui::CreateChild<Ui::RpWidget>(containerRaw);
	separator->paintRequest(
	) | rpl::on_next([=, bg = st.textBgOver] {
		auto p = QPainter(separator);
		p.fillRect(separator->rect(), bg);
	}, separator->lifetime());

	const auto separatorHeight = 2 * st.toggle.border + st.toggle.diameter;

	button->geometryValue(
	) | rpl::on_next([=](const QRect &r) {
		const auto w = st::rightsButtonToggleWidth;
		toggleButton->setGeometry(
			r.x() + r.width() - w,
			r.y(),
			w,
			r.height());
		separator->setGeometry(
			toggleButton->x() - st::lineWidth,
			r.y() + (r.height() - separatorHeight) / 2,
			st::lineWidth,
			separatorHeight);
	}, toggleButton->lifetime());

	const auto checkWidget = Ui::CreateChild<Ui::RpWidget>(toggleButton);
	checkWidget->resize(checkView->getSize());
	checkWidget->paintRequest(
	) | rpl::on_next([=] {
		auto p = QPainter(checkWidget);
		checkView->paint(p, 0, 0, checkWidget->width());
	}, checkWidget->lifetime());

	toggleButton->sizeValue(
	) | rpl::on_next([=](const QSize &s) {
		checkWidget->moveToRight(
			st.toggleSkip,
			(s.height() - checkWidget->height()) / 2);
	}, toggleButton->lifetime());

	// Slide wrap for checkboxes
	auto wrap = containerRaw->add(
		object_ptr<Ui::SlideWrap<Ui::VerticalLayout>>(
			containerRaw,
			object_ptr<Ui::VerticalLayout>(containerRaw)));

	const auto wrapRaw = wrap;
	const auto layout = wrapRaw->entity();

	wrapRaw->hide(anim::type::instant);

	containerRaw->widthValue(
	) | rpl::on_next([=](int w) {
		wrapRaw->resizeToWidth(w);
	}, wrapRaw->lifetime());

	const auto addCheckbox = [&](
		const QString &optionKey,
		rpl::producer<QString> labelText) {
		const auto checkbox = layout->add(
			object_ptr<Ui::Checkbox>(
				layout,
				std::move(labelText),
				RabbitSettings::JsonSettings::GetBool(optionKey),
				st::settingsCheckbox),
			st::settingsCheckboxPadding);

		RabbitSettings::JsonSettings::Events(optionKey)
		| rpl::on_next([=] {
			checkbox->setChecked(
				RabbitSettings::JsonSettings::GetBool(optionKey),
				Ui::Checkbox::NotifyAboutChange::DontNotify);
		}, checkbox->lifetime());

		checkbox->checkedChanges(
		) | rpl::filter([=](bool checked) {
			return checked != RabbitSettings::JsonSettings::GetBool(optionKey);
		}) | rpl::on_next([=](bool checked) {
			RabbitSettings::JsonSettings::Set(optionKey, checked);
		}, checkbox->lifetime());

		return checkbox;
	};

	const auto cbChannels = addCheckbox(u"hide_reactions_channels"_q, rktr(u"rtg_chats_hide_reactions_channels"_q));
	const auto cbGroups = addCheckbox(u"hide_reactions_groups"_q, rktr(u"rtg_chats_hide_reactions_groups"_q));
	const auto cbPrivates = addCheckbox(u"hide_reactions_privates"_q, rktr(u"rtg_chats_hide_reactions_privates"_q));

	const auto countChecked = [=] {
		auto count = 0;
		if (cbChannels->checked()) count++;
		if (cbGroups->checked()) count++;
		if (cbPrivates->checked()) count++;
		return count;
	};

	const auto updateGlobalToggle = [=] {
		checkView->setChecked(countChecked() > 0, anim::type::normal);
	};

	const auto handleCheckboxChange = [=] {
		updateGlobalToggle();
	};

	cbChannels->checkedChanges() | rpl::on_next(handleCheckboxChange, cbChannels->lifetime());
	cbGroups->checkedChanges() | rpl::on_next(handleCheckboxChange, cbGroups->lifetime());
	cbPrivates->checkedChanges() | rpl::on_next(handleCheckboxChange, cbPrivates->lifetime());

	// Initialize
	const auto initialCount = countChecked();
	wrapRaw->toggle(initialCount > 0, anim::type::instant);
	checkView->setChecked(initialCount > 0, anim::type::instant);

	// Toggle button clicks
	toggleButton->clicks(
	) | rpl::on_next([=] {
		const auto targetChecked = (countChecked() == 0);

		// Set all checkboxes
		cbChannels->setChecked(targetChecked);
		cbGroups->setChecked(targetChecked);
		cbPrivates->setChecked(targetChecked);

		updateGlobalToggle();
	}, toggleButton->lifetime());

	// Main button clicks
	button->clicks(
	) | rpl::on_next([=] {
		wrapRaw->toggle(!wrapRaw->toggled(), anim::type::normal);
	}, button->lifetime());

	// Arrow
	const auto arrow = Ui::CreateChild<Ui::RpWidget>(button);
	{
		const auto &icon = st::permissionsExpandIcon;
		arrow->resize(icon.size());
		arrow->paintRequest(
		) | rpl::on_next([=, &icon] {
			auto p = QPainter(arrow);
			const auto center = QPointF(
				icon.width() / 2.,
				icon.height() / 2.);
			const auto progress = state->animation.value(
				wrapRaw->toggled() ? 1. : 0.);
			auto hq = std::optional<PainterHighQualityEnabler>();
			if (progress > 0.) {
				hq.emplace(p);
				p.translate(center);
				p.rotate(progress * 180.);
				p.translate(-center);
			}
			icon.paint(p, 0, 0, arrow->width());
		}, arrow->lifetime());
	}

	wrapRaw->toggledValue(
	) | rpl::skip(1) | rpl::on_next([=](bool toggled) {
		state->animation.start(
			[=] { arrow->update(); },
			toggled ? 0. : 1.,
			toggled ? 1. : 0.,
			st::slideWrapDuration);
	}, button->lifetime());

	auto countCheckedProducer = rpl::combine(
		rpl::single(cbChannels->checked()) | rpl::then(cbChannels->checkedChanges()),
		rpl::single(cbGroups->checked()) | rpl::then(cbGroups->checkedChanges()),
		rpl::single(cbPrivates->checked()) | rpl::then(cbPrivates->checkedChanges())
	) | rpl::map([=](bool a, bool b, bool c) {
		return (a ? 1 : 0) + (b ? 1 : 0) + (c ? 1 : 0);
	});

	const auto label = Ui::CreateChild<Ui::FlatLabel>(
		button,
		rpl::combine(
			rktr(u"rtg_chats_hide_reactions"_q),
			std::move(countCheckedProducer)
		) | rpl::map([=](const QString &t, int checked) {
			auto count = tr::bold("  "
				+ QString::number(checked)
				+ "/3");
			return TextWithEntities::Simple(t).append(std::move(count));
		})
	);
	label->setAttribute(Qt::WA_TransparentForMouseEvents);

	const auto reposition = [=, &st] {
		const auto s = button->size();
		const auto labelLeft = st.padding.left();
		const auto labelRight = s.width() - toggleButton->width();

		const auto arrowSkip = st::rightsButtonArrowSkip;
		label->resizeToWidth(
			labelRight - labelLeft - arrow->width() - arrowSkip);
		label->moveToLeft(
			labelLeft,
			(s.height() - label->height()) / 2);
		arrow->moveToLeft(
			std::min(
				labelLeft + label->textMaxWidth() + arrowSkip,
				labelRight - arrow->width()),
			(s.height() - arrow->height()) / 2);
	};

	rpl::merge(
		button->sizeValue() | rpl::to_empty,
		cbChannels->checkedChanges() | rpl::to_empty,
		cbGroups->checkedChanges() | rpl::to_empty,
		cbPrivates->checkedChanges() | rpl::to_empty
	) | rpl::on_next([=] {
		reposition();
	}, button->lifetime());

	// Context menu
	RtgMenu::AttachSettingsContextMenu(
		button, u"rabbit/chats/hide_reactions"_q, controller);

	return container;
}

void BuildStickers(
		SectionBuilder &builder,
		Window::SessionController *controller) {
	builder.addSubsectionTitle({
		.id = u"rabbit/chats/stickers"_q,
		.title = rktr(u"rtg_chats_stickers"_q),
		.keywords = { u"stickers"_q },
	});

	AddToggle(
		builder,
		u"rabbit/chats/more_recent_stickers"_q,
		rktr(u"rtg_chats_more_recent_stickers"_q),
		nullptr,
		[] { return RabbitSettings::moreRecentStickers(); },
		[](bool value) { RabbitSettings::setMoreRecentStickers(value); },
		{ u"stickers"_q, u"recent"_q });

	if (controller) {
		builder.add([=](const WidgetContext &ctx) -> SectionBuilder::WidgetToAdd {
			return SectionBuilder::WidgetToAdd{
				.widget = CreateHideReactionsControl(ctx.container, controller),
			};
		});
	}
}

void BuildRabbitChats(
	SectionBuilder &builder,
	Window::SessionController *controller) {
	builder.addSkip();
	BuildTopGeneral(builder);

	builder.addSkip();
	builder.addDivider();
	builder.addSkip();
	BuildStickerShape(builder);

	builder.addSkip();
	builder.addDivider();
	builder.addSkip();
	BuildStickers(builder, controller);

	builder.addSkip();
	builder.addDivider();
	builder.addSkip();
	BuildQuickActions(builder, controller);

	builder.addSkip();
	builder.addDivider();
	builder.addSkip();
	BuildChats(builder);

	builder.addSkip();
	builder.addDivider();
	builder.addSkip();
	BuildMessages(builder);
}

const auto kMeta = BuildHelper({
	.id = RabbitChats::Id(),
	.parentId = Rabbit::Id(),
	.title = rktr_phrase(u"rtg_settings_chats"_q),
	.icon = &st::menuIconChatBubble,
}, [](SectionBuilder &builder) {
	BuildRabbitChats(builder, builder.controller());
});

const SectionBuildMethod kRabbitChatsSection = kMeta.build;

} // namespace

rpl::producer<QString> RabbitChats::title() {
	return rktr(u"rtg_settings_chats"_q);
}

RabbitChats::RabbitChats(
	QWidget *parent,
	not_null<Window::SessionController *> controller)
	: Section(parent, controller) {
	setupContent();
}

void RabbitChats::setupContent() {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
	build(content, kRabbitChatsSection);
	Ui::ResizeFitChild(this, content);
}

} // namespace Settings
