//  Copyright (c) 2015 Hugo Amiard hugo.amiard@laposte.net
//  This software is provided 'as-is' under the zlib License, see the LICENSE.txt file.
//  This notice and the license may not be removed or altered from any source distribution.

#ifndef MK_NANOINK_H
#define MK_NANOINK_H

/* mk */
#include <Ui/mkUiForward.h>
#include <Ui/Frame/mkInk.h>
#include <Ui/Nano/nanovg/nanovg.h>

namespace mk
{
	class NanoInk : public Inkbox
	{
	public:
		NanoInk(Frame& frame, NanoLayer& layer);
		~NanoInk();

		void updateContent();
		void updateFrame();
		void updateStyle();
		
		void show();
		void hide();

		float contentSize(Dimension dim);
		size_t caretIndex(float x, float y);
		void caretCoords(size_t index, float& caretX, float& caretY, float& caretHeight);

		int fetchImage(const string& image, bool tile = false);
		void drawImage(int image, float x, float y, float w, float h);
		void drawSkinImage(ImageSkin::Section section, float x, float y, float w, float h);

		void splitImageSkin(int image);

		void setupText();

		void nanodraw();

	protected:
		NVGcontext* mCtx;
		NanoLayer& mLayer;
		BoxFloat mCorners;
		int mImage;
		int mOverlay;
		int mTile;
		int mSkin;
		std::vector<NVGtextRow> mTextRows;
	};

}

#endif