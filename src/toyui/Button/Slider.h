//  Copyright (c) 2016 Hugo Amiard hugo.amiard@laposte.net
//  This software is provided 'as-is' under the zlib License, see the LICENSE.txt file.
//  This notice and the license may not be removed or altered from any source distribution.

#ifndef TOY_SLIDER_H
#define TOY_SLIDER_H

/* toy */
#include <toyobj/Util/Stat.h>
#include <toyobj/String/StringConvert.h>
#include <toyui/Forward.h>
#include <toyui/Frame/UiRect.h>
#include <toyui/Edit/Value.h>
#include <toyui/Widget/Widget.h>
#include <toyui/Button/Button.h>
#include <toyui/Edit/TypeIn.h>

namespace toy
{
	class _refl_ TOY_UI_EXPORT Slider : public Wedge
	{
	public:
		Slider(Wedge& parent, Dimension dim = DIM_X, const Callback& onUpdated = nullptr, Type& type = cls());

		Widget& filler() { return m_filler; }
		Item& knob() { return m_button; }

		float val() { return m_val; }

		virtual void dirtyLayout();

		void updateMetrics(float min, float max, float val, float stepLength, float knobLength = 0.f);
		void resetMetrics(float min, float max, float val, float stepLength, float knobLength = 0.f);

		void offsetChange(float offset, bool ended);
		
		void updateKnob();

		virtual void sliderStep(float value, bool ended) { UNUSED(value); UNUSED(ended); m_onUpdated(*this); }

		float offset(MouseEvent& mouseEvent);

		virtual bool leftClick(MouseEvent& mouseEvent);

		virtual bool leftDragStart(MouseEvent& mouseEvent);
		virtual bool leftDrag(MouseEvent& mouseEvent);
		virtual bool leftDragEnd(MouseEvent& mouseEvent);

		static Type& cls() { static Type ty("Slider", Wedge::WrapControl()); return ty; }

		static Type& Knob() { static Type ty("SliderKnob", Item::cls()); return ty; }
		static Type& Display() { static Type ty("SliderDisplay", Label::cls()); return ty; }

	protected:
		Dimension m_dim;
		Item m_filler;
		Item m_button;
		Item m_spacer;

		float m_min;
		float m_max;
		float m_range;
		float m_val;
		float m_stepLength;
		float m_knobLength;

		float m_numSteps;
		int m_step;

		Callback m_onUpdated;
	};
}

#endif
