//  Copyright (c) 2015 Hugo Amiard hugo.amiard@laposte.net
//  This software is provided 'as-is' under the zlib License, see the LICENSE.txt file.
//  This notice and the license may not be removed or altered from any source distribution.

#include <Ui/mkUiConfig.h>
#include <Ui/Nano/mkNanoInk.h>

#include <Object/Util/mkColour.h>
#include <Ui/Frame/mkFrame.h>
#include <Ui/Frame/mkStripe.h>
#include <Ui/Widget/mkWidget.h>

#include <Ui/Nano/mkNanoWindow.h>

#include <Ui/Nano/nanovg/nanovg.h>

#include <cmath>

#include <Ui/Widget/mkSheet.h>

namespace mk
{
	inline float clamp(float v, float mn, float mx)
	{
		return (v > mx) ? mx : (v < mn) ? mn : v;
	}

	static float fminf(float a, float b)
	{
		return isnan(a) ? b : (isnan(b) ? a : ((a < b) ? a : b));
	}

	static float fmaxf(float a, float b)
	{
		return isnan(a) ? b : (isnan(b) ? a : ((a > b) ? a : b));
	}

	static double fmin(double a, double b)
	{
		return isnan(a) ? b : (isnan(b) ? a : ((a < b) ? a : b));
	}

	static double fmax(double a, double b)
	{
		return isnan(a) ? b : (isnan(b) ? a : ((a > b) ? a : b));
	}

	void nvgRoundedBox(NVGcontext *ctx, float x, float y, float w, float h, float cr0, float cr1, float cr2, float cr3)
	{
		float d;

		w = fmaxf(0, w);
		h = fmaxf(0, h);
		d = fminf(w, h);

		nvgMoveTo(ctx, x, y + h*0.5f);
		nvgArcTo(ctx, x, y, x + w, y, fminf(cr0, d / 2));
		nvgArcTo(ctx, x + w, y, x + w, y + h, fminf(cr1, d / 2));
		nvgArcTo(ctx, x + w, y + h, x, y + h, fminf(cr2, d / 2));
		nvgArcTo(ctx, x, y + h, x, y, fminf(cr3, d / 2));
		nvgClosePath(ctx);
	}

	NVGcolor nvgColour(const Colour& colour)
	{
		return nvgRGBAf(colour.r(), colour.g(), colour.b(), colour.a());
	}

	NVGcolor nvgOffsetColour(const Colour& colour, float delta)
	{
		float offset = delta / 255.0f;
		return nvgRGBAf(	clamp(colour.r() + offset, 0, 1),
							clamp(colour.g() + offset, 0, 1),
							clamp(colour.b() + offset, 0, 1),
							colour.a());
	}

	NanoInk::NanoInk(Frame& frame, NanoLayer& layer)
		: Inkbox(frame)
		, mCtx(layer.target().window().ctx())
		, mLayer(layer)
		, mImage(0)
	{}

	NanoInk::~NanoInk()
	{}

	void NanoInk::show()
	{
		mVisible = true;
	}

	void NanoInk::hide()
	{
		mVisible = false;
	}

	void NanoInk::nanodraw()
	{
		//std::cerr << "ink :: draw " << mFrame.style()->name() << std::endl;

		if(skin().mEmpty || !mVisible || mFrame.dclip(DIM_Y) == Frame::HIDDEN || mFrame.dclip(DIM_X) == Frame::HIDDEN)
			return;

		//nvgSave(mCtx);

		float left = mFrame.cleft();
		float top = mFrame.ctop();
		float width = mFrame.cwidth();
		float height = mFrame.cheight();;

		float pleft = mFrame.pleft();
		float ptop = mFrame.ptop();
		float pwidth = mFrame.pwidth();
		float pheight = mFrame.pheight();

		float contentWidth = contentSize(DIM_X) - skin().padding()[DIM_X] - skin().padding()[DIM_X + 2];
		float contentHeight = contentSize(DIM_Y) - skin().padding()[DIM_Y] - skin().padding()[DIM_Y + 2];

		float cleft = pleft;
		NVGalign halign = NVG_ALIGN_LEFT;
		if(skin().align()[DIM_X] == CENTER)
		{
			halign = NVG_ALIGN_CENTER;
			cleft = pleft + pwidth / 2.f - contentWidth / 2.f;
		}
		else if(skin().align()[DIM_X] == RIGHT)
		{
			halign = NVG_ALIGN_RIGHT;
			cleft = pleft + pwidth - contentWidth;
		}

		float ctop = ptop;
		if(skin().align()[DIM_Y] == CENTER)
			ctop = ptop + pheight / 2.f - contentHeight / 2.f;
		else if(skin().align()[DIM_Y] == RIGHT)
			ctop = ptop + pheight - contentHeight;

		float c0 = mCorners.x0();
		float c1 = mCorners.y0();
		float c2 = mCorners.x1();
		float c3 = mCorners.y1();

		//std::cerr << "ink :: draw " << mFrame.style()->name() << " at " << left << " , " << top << " size " << width << " , " << height << std::endl;

		// Rect

		if(mFrame.dclip(DIM_X) || mFrame.dclip(DIM_Y))
			nvgScissor(mCtx, left, top, width, height);

		nvgBeginPath(mCtx);

		if(mCorners.null())
			nvgRect(mCtx, left, top, width, height);
		else
			nvgRoundedBox(mCtx, left, top, width, height, c0, c1, c2, c3);

		if(skin().backgroundColour().a() != 0.f)
		{
			if(skin().topdownGradient().null())
			{
				nvgFillColor(mCtx, nvgColour(skin().mBackgroundColour));
			}
			else
			{
				NVGcolor first = nvgOffsetColour(skin().backgroundColour(), skin().topdownGradient().x());
				NVGcolor second = nvgOffsetColour(skin().backgroundColour(), skin().topdownGradient().y());
				nvgFillPaint(mCtx, (height > width) ?
					nvgLinearGradient(mCtx, left, top, left + width, top, first, second) :
					nvgLinearGradient(mCtx, left, top, left, top + height, first, second));
			}
			nvgFill(mCtx);
		}
		
		if(skin().borderWidth().x0() > 0.f)
		{
			nvgBeginPath(mCtx);
			nvgRoundedBox(mCtx, left + 0.5f, top + 0.5f, width - 1.f, height - 1.f, c0, c1, c2, c3);
			//nvgRoundedRect(mCtx, left + 0.5f, top + 0.5f, width - 1.f, height - 1.f, skin().mCornerRadius.x());
			nvgStrokeWidth(mCtx, skin().borderWidth().x0());
			nvgStrokeColor(mCtx, nvgColour(skin().borderColour()));
			nvgStroke(mCtx);
		}

		// Image
		if(mImage || mOverlay)
		{
			nvgBeginPath(mCtx);
			if(mCorners.null())
				nvgRect(mCtx, cleft, ctop, contentWidth, contentHeight);
			else
				nvgRoundedBox(mCtx, cleft, ctop, contentWidth, contentHeight, c0, c1, c2, c3);

			if(mImage)
			{
				NVGpaint imgPaint = nvgImagePattern(mCtx, cleft, ctop, contentWidth, contentHeight, 0.0f / 180.0f*NVG_PI, mImage, 1.f);
				nvgFillPaint(mCtx, imgPaint);
				nvgFill(mCtx);
			}
			if(mOverlay)
			{
				NVGpaint imgPaint = nvgImagePattern(mCtx, cleft, ctop, contentWidth, contentHeight, 0.0f / 180.0f*NVG_PI, mOverlay, 1.f);
				nvgFillPaint(mCtx, imgPaint);
				nvgFill(mCtx);
			}
		}

		// ImageSkin
		if(!skin().imageSkin().null())
		{
			const ImageSkin& imgskin = skin().mImageSkin;
			float margin = skin().imageSkin().d_margin * 2.f;
			imgskin.stretchCoords(width + margin, height + margin, [this, left, top](ImageSkin::Section s, int x, int y, int w, int h){ this->drawSkinImage(s, float(left + x), float(top + y), float(w), float(h)); });
		}

		// Caption
		if(!mFrame.widget().label().empty() && !(pwidth <= 0.f || pheight <= 0.f))
		{
			this->setupText();

			//if(mFrame.dclip(DIM_X) || mFrame.dclip(DIM_Y)) 
			// ^ @note this doesn't work because a frame is set to clipped only by its parent, and not when the label is larger than the frame itself
			nvgScissor(mCtx, left, top, width, height);

			float lineh = 0.f;
			nvgTextMetrics(mCtx, NULL, NULL, &lineh);

			const char* start = mFrame.widget().label().c_str();
			float x = pleft;
			float y = ptop;

			for(NVGtextRow& row : mTextRows)
			{
				if(halign & NVG_ALIGN_LEFT)
					x = pleft;
				else if(halign & NVG_ALIGN_CENTER)
					x = pleft + pwidth*0.5f - row.width*0.5f;
				else if(halign & NVG_ALIGN_RIGHT)
					x = pleft + pwidth - row.width;
						
				if(mSelectFirst != mSelectSecond)
				{
					size_t indexStart = row.start - start;
					size_t indexEnd = row.end - start;

					if(indexEnd > selectStart() && indexStart < selectEnd())
					{
						size_t selectStart = std::max(indexStart, this->selectStart());
						size_t selectEnd = std::min(indexEnd, this->selectEnd());

						NVGglyphPosition startPosition;
						nvgTextGlyphPosition(mCtx, 0.f, 0.f, row.start, row.end, selectStart - (row.start - start), &startPosition);

						NVGglyphPosition endPosition;
						nvgTextGlyphPosition(mCtx, 0.f, 0.f, row.start, row.end, selectEnd - (row.start - start), &endPosition);

						nvgBeginPath(mCtx);
						nvgFillColor(mCtx, nvgRGBA(0, 55, 255, 124));
						nvgRect(mCtx, x + startPosition.x, y, endPosition.x - startPosition.x, lineh);
						nvgFill(mCtx);
					}
				}

				nvgFillColor(mCtx, nvgColour(skin().mTextColour));
				nvgText(mCtx, x, y, row.start, row.end);
					
				y += lineh;
			}

			/*
			nvgText(mCtx, cleft, ctop, mFrame.widget().label().c_str(), nullptr);
			*/

			//if(mFrame.dclip(DIM_X) || mFrame.dclip(DIM_Y))
				nvgResetScissor(mCtx);
		}

		if(mFrame.dclip(DIM_X) || mFrame.dclip(DIM_Y))
			nvgResetScissor(mCtx);
		
		//nvgRestore(mCtx);
	}

	float NanoInk::contentSize(Dimension dim)
	{
		if(mImage)
		{
			int width, height;
			nvgImageSize(mCtx, mImage, &width, &height);

			float xoffset = skin().padding()[DIM_X] + skin().padding()[DIM_X + 2];
			float yoffset = skin().padding()[DIM_Y] + skin().padding()[DIM_Y + 2];

			return dim == DIM_X ? float(width) + xoffset : float(height) + yoffset;
		}
		else if(!mFrame.widget().label().empty())
		{
			float bounds[4];
			float height;

			nvgTextBounds(mCtx, 0.f, 0.f, mFrame.widget().label().c_str(), nullptr, bounds);
			nvgTextMetrics(mCtx, nullptr, nullptr, &height);

			float xoffset = skin().padding()[DIM_X] + skin().padding()[DIM_X + 2];
			float yoffset = skin().padding()[DIM_Y] + skin().padding()[DIM_Y + 2];

			//std::cerr << "ink :: contentSize for " << mFrame.style()->name() << " : " << (dim == DIM_X ? " x " : " y ") << (dim == DIM_X ? bounds[2] - bounds[0] + xoffset : bounds[3] - bounds[1] + yoffset) << std::endl;

			return dim == DIM_X ? bounds[2] - bounds[0] + xoffset : height + yoffset;
		}
		
		return 0.f;
	}

	size_t NanoInk::caretIndex(float posX, float posY)
	{
		const char* start = mFrame.widget().label().c_str();
		const char* end = start + mFrame.widget().label().size();

		float pwidth = mFrame.pwidth();
		float x = skin().padding()[DIM_X];
		float y = skin().padding()[DIM_Y];

		float lineh = 0.f;
		nvgTextMetrics(mCtx, NULL, NULL, &lineh);

		for(NVGtextRow& row : mTextRows)
		{
			if(posY < y + lineh)
			{
				NVGalign halign = NVG_ALIGN_LEFT;
				if(halign & NVG_ALIGN_LEFT)
					x = x;
				else if(halign & NVG_ALIGN_CENTER)
					x = x + pwidth*0.5f - row.width*0.5f;
				else if(halign & NVG_ALIGN_RIGHT)
					x = x + pwidth - row.width;

				return nvgTextGlyphIndex(mCtx, x, y, row.start, row.end, posX) + row.start - start;
			}
			y += lineh;
		}

		return end - start;

		//return nvgTextGlyphIndex(mCtx, skin().padding()[DIM_X], skin().padding()[DIM_Y], mFrame.widget().label().c_str(), nullptr, posX);
	}

	void NanoInk::caretCoords(size_t index, float& caretX, float& caretY, float& caretHeight)
	{
		float lineh = 0.f;
		nvgTextMetrics(mCtx, NULL, NULL, &lineh);

		const char* start = mFrame.widget().label().c_str();

		float x = skin().padding()[DIM_X];
		float y = skin().padding()[DIM_Y];
		float pwidth = mFrame.pwidth();

		for(NVGtextRow& row : mTextRows)
		{
			if(index <= row.end - start)
			{
				NVGglyphPosition position;
				nvgTextGlyphPosition(mCtx, 0.f, 0.f, row.start, row.end, index - (row.start - start), &position);
				caretX = x + position.x;
				caretY = y;
				nvgTextMetrics(mCtx, nullptr, nullptr, &caretHeight);
				return;
			}
			y += lineh;
		}

		caretX = 0.f;
		caretY = y;
		return;

		NVGglyphPosition position;
		nvgTextGlyphPosition(mCtx, 0.f, 0.f, mFrame.widget().label().c_str(), nullptr, index, &position);
		caretX = skin().padding()[DIM_X] + position.x;
		caretY = skin().padding()[DIM_Y] + 0.f;
		nvgTextMetrics(mCtx, nullptr, nullptr, &caretHeight);
	}

	void NanoInk::updateContent()
	{}

	void NanoInk::updateStyle()
	{
		mCorners = skin().mCornerRadius;

		if(skin().mEmpty)
			return;

		if(!mFrame.widget().image().empty())
			mImage = fetchImage(mFrame.widget().image());
		else
			mImage = 0;

		if(!skin().overlay().empty())
			mOverlay = fetchImage(skin().overlay());
		else
			mOverlay = 0;

		if(!skin().imageSkin().null())
		{
			mSkin = fetchImage(skin().imageSkin().d_image);

			if(!skin().imageSkin().d_prepared)
			{
				int imgwidth, imgheight;
				nvgImageSize(mCtx, mSkin, &imgwidth, &imgheight);
				skin().imageSkin().prepare(imgwidth, imgheight);
			}
		}
	}

	int NanoInk::fetchImage(const string& image)
	{
		auto it = NanoWindow::sImages.find(image);
		if(it != NanoWindow::sImages.end())
			return (*it).second;

		NanoWindow::sImages[image] = nvgCreateImage(mCtx, (mLayer.target().window().resourcePath() + "interface/uisprites/" + image + ".png").c_str(), 0);
		return NanoWindow::sImages[image];
	}

	void NanoInk::setupText()
	{
		nvgFontSize(mCtx, skin().textSize());
		nvgFontFace(mCtx, skin().textFont().c_str());
		nvgTextAlign(mCtx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
	}

	void NanoInk::updateFrame()
	{
		if(skin().mEmpty || !mVisible || mFrame.dsize(DIM_X) == 0.f || mFrame.dsize(DIM_X) == 0.f)
			return;

		if(!mFrame.widget().label().empty())
		{
			mTextRows.clear();

			this->setupText();

			const char* first = mFrame.widget().label().c_str();
			const char* end = first + mFrame.widget().label().size();
			int nrows = 0;

			mTextRows.resize(1);

			while((nrows = nvgTextBreakLines(mCtx, first, end, mFrame.pwidth(), &mTextRows.back(), 1)))
			{
				first = mTextRows.back().next;
				mTextRows.resize(mTextRows.size() + 1);
			}

			mTextRows.pop_back();
		}

		NanoInk& parent = static_cast<NanoInk&>(mFrame.parent()->inkbox());
		if(parent.mCorners.null() || !skin().mWeakCorners)
			return;

		if(mFrame.parent()->layoutDim() == DIM_X)
		{
			mCorners.setX0(mFrame.first() ? parent.mCorners.x0() : 0.f);
			mCorners.setY1(mFrame.first() ? parent.mCorners.y1() : 0.f);
			mCorners.setY0(mFrame.last() ? parent.mCorners.y0() : 0.f);
			mCorners.setX1(mFrame.last() ? parent.mCorners.x1() : 0.f);
		}
		else if(mFrame.parent()->layoutDim() == DIM_Y)
		{
			mCorners.setX0(mFrame.first() ? parent.mCorners.x0() : 0.f);
			mCorners.setY0(mFrame.first() ? parent.mCorners.y0() : 0.f);
			mCorners.setX1(mFrame.last() ? parent.mCorners.x1() : 0.f);
			mCorners.setY1(mFrame.last() ? parent.mCorners.y1() : 0.f);
		}
	}

	void NanoInk::drawImage(int image, float left, float top, float width, float height)
	{
		NVGpaint imgPaint = nvgImagePattern(mCtx, left, top, width, height, 0.0f / 180.0f*NVG_PI, image, 1.f);
		nvgBeginPath(mCtx);
		nvgRect(mCtx, left, top, width, height);
		nvgFillPaint(mCtx, imgPaint);
		nvgFill(mCtx);
	}

	void NanoInk::drawSkinImage(ImageSkin::Section section, float left, float top, float width, float height)
	{
		int imgwidth, imgheight;
		nvgImageSize(mCtx, mSkin, &imgwidth, &imgheight);

		left -= skin().imageSkin().d_margin;
		top -= skin().imageSkin().d_margin;

		float xoffset = -skin().imageSkin().d_coords[section].x0();
		float yoffset = -skin().imageSkin().d_coords[section].y0();

		if(section == ImageSkin::TOP || section == ImageSkin::BOTTOM || section == ImageSkin::FILL)
		{
			float ratio = width / skin().imageSkin().d_fillWidth; //float(imgwidth);
			xoffset *= ratio;
			imgwidth *= ratio;
		}
		if(section == ImageSkin::LEFT || section == ImageSkin::RIGHT || section == ImageSkin::FILL)
		{
			float ratio = height / skin().imageSkin().d_fillHeight; //float(imgheight);
			yoffset *= ratio;
			imgheight *= ratio;
		}

		NVGpaint imgPaint = nvgImagePattern(mCtx, left + xoffset, top + yoffset, imgwidth, imgheight, 0.0f / 180.0f*NVG_PI, mSkin, 1.f);
		nvgBeginPath(mCtx);
		nvgRect(mCtx, left, top, width, height);
		nvgFillPaint(mCtx, imgPaint);
		nvgFill(mCtx);
	}
}