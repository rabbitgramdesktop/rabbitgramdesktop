/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "ui/effects/premium_star_model.h"

#include "ui/effects/premium_3d_mesh.h"

#include <QtCore/QString>

namespace Ui::Premium {

StarModel LoadStarModel() {
	auto mesh = LoadObject3dMesh(
		u":/gui/art/premium/star.binobj"_q,
		1.f,
		true);
	auto result = StarModel();
	result.vertices = std::move(mesh.vertices);
	result.vertexCount = mesh.vertexCount;
	return result;
}

} // namespace Ui::Premium
