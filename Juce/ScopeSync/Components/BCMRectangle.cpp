/**
 * A simple Rectangle class, for drawing onto a Component
 *
 *  (C) Copyright 2014 bcmodular (http://www.bcmodular.co.uk/)
 *
 * This file is part of ScopeSync.
 *
 * ScopeSync is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * ScopeSync is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ScopeSync.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contributors:
 *  Simon Russell
 *  Will Ellis
 *  Jessica Brandt
 */

#include "BCMRectangle.h"

BCMRectangle::BCMRectangle(XmlElement& xml) : BCMGraphic(xml)
{
    cornerSize       = static_cast<float>(xml.getDoubleAttribute("cornersize", 0.0f));
    fillColour       = xml.getStringAttribute("fillcolour", "00000000");
    outlineThickness = static_cast<float>(xml.getDoubleAttribute("outlinethickness", 0.0f));
    outlineColour    = xml.getStringAttribute("outlinecolour", "00000000");
}

BCMRectangle::~BCMRectangle() {}