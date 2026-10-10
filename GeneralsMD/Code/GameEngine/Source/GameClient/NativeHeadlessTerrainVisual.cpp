// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "GameClient/NativeHeadlessTerrainVisual.h"

void NativeHeadlessTerrainVisual::terrainHeightsChanged() noexcept {}
void NativeHeadlessTerrainVisual::getTerrainColorAt(Real,Real,RGBColor*) {throw ERROR_BAD_ARG;}
void NativeHeadlessTerrainVisual::setTerrainTracksDetail() {throw ERROR_BAD_ARG;}
void NativeHeadlessTerrainVisual::setShoreLineDetail() {throw ERROR_BAD_ARG;}
void NativeHeadlessTerrainVisual::addFactionBib(Object*,Bool,Real) {throw ERROR_BAD_ARG;}
void NativeHeadlessTerrainVisual::removeFactionBib(Object*) {throw ERROR_BAD_ARG;}
void NativeHeadlessTerrainVisual::addFactionBibDrawable(Drawable*,Bool,Real) {throw ERROR_BAD_ARG;}
void NativeHeadlessTerrainVisual::removeFactionBibDrawable(Drawable*) {throw ERROR_BAD_ARG;}
void NativeHeadlessTerrainVisual::removeAllBibs() {}
void NativeHeadlessTerrainVisual::removeBibHighlighting() {}
void NativeHeadlessTerrainVisual::removeTreesAndPropsForConstruction(const Coord3D*,const GeometryInfo&,Real) {throw ERROR_BAD_ARG;}
void NativeHeadlessTerrainVisual::addProp(const ThingTemplate*,const Coord3D*,Real) {throw ERROR_BAD_ARG;}
void NativeHeadlessTerrainVisual::replaceSkyboxTextures(const AsciiString* [5],const AsciiString* [5]) {throw ERROR_BAD_ARG;}
void NativeHeadlessTerrainVisual::xfer(Xfer*) {throw ERROR_BAD_ARG;}
