/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/settings_menu/sections/rabbit_other.h"

#include <rpl/rpl.h>
#include <QtGui/QPainter>
#include "data/data_session.h"
#include "main/main_session.h"
#include "rabbit/lang/rabbit_lang.h"
#include "rabbit/settings/rabbit_settings.h"
#include "rabbit/settings_menu/rabbit_context_menu.h"
#include "rabbit/settings_menu/rabbit_settings_menu.h"
#include "settings/settings_builder.h"
#include "settings/settings_common.h"
#include "storage/localstorage.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/checkbox.h"
#include "ui/wrap/slide_wrap.h"
#include "ui/wrap/vertical_layout.h"
#include "ui/painter.h"
#include "window/window_session_controller.h"
#include "mainwindow.h"

#include "styles/style_menu_icons.h"
#include "styles/style_settings.h"
#include "styles/style_boxes.h"
#include "styles/style_info.h"
#include "styles/style_edit_peer_members.h"
#include "ui/text/text_utilities.h"
#include "lang/lang_keys.h"

namespace Settings {
namespace {

using namespace Builder;

void AddJsonToggle(
	SectionBuilder &builder,
	QString id,
	const QString &titleKey,
	const QString &optionKey,
	QStringList keywords = {}) {
	const auto current = RabbitSettings::JsonSettings::GetBool(optionKey);
	const auto controlId = id;
	if (const auto button = builder.addButton({
		.id = std::move(id),
		.title = rktr(titleKey),
		.st = &st::settingsButtonNoIcon,
		.toggled = rpl::single(current),
		.keywords = std::move(keywords),
	})) {
		button->toggledValue()
			| rpl::filter([=](bool enabled) {
				return enabled != RabbitSettings::JsonSettings::GetBool(optionKey);
			})
			| rpl::on_next([=](bool enabled) {
				RabbitSettings::JsonSettings::Set(optionKey, enabled);
			}, button->lifetime());
		if (const auto controller = builder.controller()) {
			RtgMenu::AttachSettingsContextMenu(
				button, controlId, controller);
		}
	}
}

object_ptr<Ui::RpWidget> CreateTelemetryControl(
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

	const auto cbAppVersion = addCheckbox(u"telemetry_app_version"_q, rktr(u"rtg_telemetry_app_version"_q));
	const auto cbOs = addCheckbox(u"telemetry_os"_q, rktr(u"rtg_telemetry_os"_q));
	const auto cbFeatures = addCheckbox(u"telemetry_features"_q, rktr(u"rtg_telemetry_features"_q));
	const auto cbCpuArch = addCheckbox(u"telemetry_cpu_arch"_q, rktr(u"rtg_telemetry_cpu_arch"_q));

	const auto countChecked = [=] {
		auto count = 0;
		if (cbAppVersion->checked()) count++;
		if (cbOs->checked()) count++;
		if (cbFeatures->checked()) count++;
		if (cbCpuArch->checked()) count++;
		return count;
	};

	const auto updateGlobalToggle = [=] {
		const auto count = countChecked();
		const auto active = (count > 0) && RabbitSettings::analytics();
		checkView->setChecked(active, anim::type::normal);
	};

	const auto handleCheckboxChange = [=](bool checked) {
		updateGlobalToggle();
		if (!checked) {
			if (countChecked() == 0) {
				RabbitSettings::setAnalytics(false);
			}
		}
	};

	cbAppVersion->checkedChanges() | rpl::on_next(handleCheckboxChange, cbAppVersion->lifetime());
	cbOs->checkedChanges() | rpl::on_next(handleCheckboxChange, cbOs->lifetime());
	cbFeatures->checkedChanges() | rpl::on_next(handleCheckboxChange, cbFeatures->lifetime());
	cbCpuArch->checkedChanges() | rpl::on_next(handleCheckboxChange, cbCpuArch->lifetime());

	// Whenever analytics changes, expand/collapse wrap
	RabbitSettings::JsonSettings::Events(u"analytics"_q)
	| rpl::on_next([=] {
		const auto enabled = RabbitSettings::analytics();
		wrapRaw->toggle(enabled, anim::type::normal);
		updateGlobalToggle();
	}, containerRaw->lifetime());

	// Initialize
	const auto initialEnabled = RabbitSettings::analytics();
	wrapRaw->toggle(false, anim::type::instant);
	checkView->setChecked(initialEnabled && (countChecked() > 0), anim::type::instant);

	// Toggle button clicks
	toggleButton->clicks(
	) | rpl::on_next([=] {
		const auto targetChecked = !checkView->checked();
		
		// Set all checkboxes
		cbAppVersion->setChecked(targetChecked);
		cbOs->setChecked(targetChecked);
		cbFeatures->setChecked(targetChecked);
		cbCpuArch->setChecked(targetChecked);

		// Update analytics setting
		RabbitSettings::setAnalytics(targetChecked);
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
		rpl::single(cbAppVersion->checked()) | rpl::then(cbAppVersion->checkedChanges()),
		rpl::single(cbOs->checked()) | rpl::then(cbOs->checkedChanges()),
		rpl::single(cbFeatures->checked()) | rpl::then(cbFeatures->checkedChanges()),
		rpl::single(cbCpuArch->checked()) | rpl::then(cbCpuArch->checkedChanges())
	) | rpl::map([=](bool a, bool b, bool c, bool d) {
		return (a ? 1 : 0) + (b ? 1 : 0) + (c ? 1 : 0) + (d ? 1 : 0);
	});

	const auto label = Ui::CreateChild<Ui::FlatLabel>(
		button,
		rpl::combine(
			rktr(u"rtg_other_analytics"_q),
			std::move(countCheckedProducer)
		) | rpl::map([=](const QString &t, int checked) {
			auto count = tr::bold("  "
				+ QString::number(checked)
				+ "/4");
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
		cbAppVersion->checkedChanges() | rpl::to_empty,
		cbOs->checkedChanges() | rpl::to_empty,
		cbFeatures->checkedChanges() | rpl::to_empty,
		cbCpuArch->checkedChanges() | rpl::to_empty
	) | rpl::on_next([=] {
		reposition();
	}, button->lifetime());

	// Context menu
	RtgMenu::AttachSettingsContextMenu(
		button, u"rabbit/other/analytics"_q, controller);

	return container;
}

void BuildOther(SectionBuilder &builder) {
	builder.addSubsectionTitle({
		.id = u"rabbit/other/other"_q,
		.title = rktr(u"rtg_settings_other"_q),
		.keywords = { u"other"_q },
	});

	builder.add([=](const WidgetContext &ctx) -> SectionBuilder::WidgetToAdd {
		auto control = CreateTelemetryControl(ctx.container, ctx.controller);
		return SectionBuilder::WidgetToAdd{
			.widget = std::move(control),
		};
	});
}

void BuildRabbitOther(SectionBuilder &builder) {
	builder.addSkip();
	BuildOther(builder);
}

const auto kMeta = BuildHelper({
	.id = RabbitOther::Id(),
	.parentId = Rabbit::Id(),
	.title = rktr_phrase(u"rtg_settings_other"_q),
	.icon = &st::menuIconShowAll,
}, [](SectionBuilder &builder) {
	BuildRabbitOther(builder);
});

const SectionBuildMethod kRabbitOtherSection = kMeta.build;

} // namespace

rpl::producer<QString> RabbitOther::title() {
	return rktr(u"rtg_settings_other"_q);
}

RabbitOther::RabbitOther(
	QWidget *parent,
	not_null<Window::SessionController *> controller)
	: Section(parent, controller) {
	setupContent();
}

void RabbitOther::setupContent() {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
	build(content, kRabbitOtherSection);
	Ui::ResizeFitChild(this, content);
}

} // namespace Settings
