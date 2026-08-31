#include "Dialog.h"

Dialog* gCurrentDialog;Dialog* CurrentDialog() {return gCurrentDialog;}
Font* CurrentDialogFont() {return gCurrentDialog->mFont;}
Color CurrentDialogFontColor() {return gCurrentDialog->mFontColor;}
Color CurrentDialogClickableColor() {return gCurrentDialog->mClickableColor;}
Point CurrentWidgetBorder() {return gCurrentDialog->mWidgetBorder;}
DialogWidgetBundle& CurrentBundle() {return *gCurrentDialog->mBundle;}

int gDialogDragMargin=5;
void SetDialogDragMargin(int theAmount) {gDialogDragMargin=theAmount;}

Dialog::Dialog()
{
	SetCPUName(this,"Dialog")
	mPanelArea.mWidth=-1;
	mFont=NULL;
	mMovePanel=0;
	mTitleFont=NULL;
	mMovePanelProgress=0;
	mTiny=false;
	mNoKill=false;

	mCurrentPanel=mNextPanel=NULL;
	mBundle=NULL;

	mDisableMagicCursor=false;

	FocusBackButton();
}

Dialog::~Dialog()
{
	if (mBundle) mBundle->Unload();
}

void Dialog::Back()
{
	if (mBack.mFeatures&CPU_DRAW) Notify(&mBack);
	else Notify(&mDone);
}

void Dialog::Initialize()
{
	if (!mTiny)
	{
		Point aDoneButtonPos=Point(mPanelArea.mX+50,mPanelArea.mY+mPanelArea.mHeight+10);
		mDone.Size(aDoneButtonPos.mX,aDoneButtonPos.mY,mWidth-(aDoneButtonPos.mX*2),mHeight-(aDoneButtonPos.mY+10));
		mDone.SetNotify(this);
		*this+=&mDone;
	
		mBack.Size(0,0,mPanelArea.mY,mPanelArea.mY);
		mBack.SetNotify(this);
		mBack.NotifyOnPress();
		*this+=&mBack;
	}

	SetupButtons();

	mBack.Hide(true);

	mScrollSpeed=mWidth/20;
	if (mCurrentPanel) mCurrentPanel->Focus();

#ifdef _MAGICCURSOR
	if (mDisableMagicCursor)
	{
		EnumList(DialogPanel,aDP,mPanelList) RemoveMagicCursorFromStack(&aDP->mMCursor);
	}
#endif


}

void Dialog::Update()
{
	if (mMovePanel)
	{
		if (mMovePanel<0)
		{
			mCurrentPanel->mWidth-=mScrollSpeed;
			mCurrentPanel->mDrawOffset.mX-=mScrollSpeed;
			mMovePanelProgress=mCurrentPanel->mWidth/mPanelArea.mWidth;
			if (mNextPanel)
			{
				mNextPanel->mX-=mScrollSpeed;
				mNextPanel->mWidth+=mScrollSpeed;
			}
			if (mCurrentPanel->mWidth<=0)
			{
				mCurrentPanel->mWidth=0;
				mCurrentPanel->Hide(true);

				if (mNextPanel)
				{
					mNextPanel->mX=mPanelArea.mX;
					mNextPanel->mWidth=mPanelArea.mWidth;
				}

				mMovePanel=0;

				mPanelStack.Push(mCurrentPanel);
				mCurrentPanel=mNextPanel;
				mCurrentPanel->BindSpinsV();
				mBack.Hide(false);
			}
		}
		else if (mMovePanel>0)
		{
			mCurrentPanel->mWidth+=mScrollSpeed;
			mCurrentPanel->mDrawOffset.mX+=mScrollSpeed;
			mMovePanelProgress=mCurrentPanel->mWidth/mPanelArea.mWidth;

			if (mNextPanel) {mNextPanel->mX+=mScrollSpeed;mNextPanel->mWidth=_max(0,mNextPanel->mWidth-mScrollSpeed);}
			if (mCurrentPanel->mWidth>=mPanelArea.mWidth)
			{
				mCurrentPanel->mWidth=mPanelArea.mWidth;
				mCurrentPanel->mDrawOffset.mX=0;
				if (mNextPanel)
				{
					mNextPanel->mX=0;
					mNextPanel->mWidth=0;
					mNextPanel->Hide(true);

					mMovePanel=0;
				}
			}
		}
	}
}

void Dialog::Notify(void* theData)
{
	mProcessedNotify=false;
	FocusKeyboard(false);
	if (theData==&mDone) {mProcessedNotify=true;if (Done()) if (!mNoKill) Kill();}
	if (theData==&mBack) {mProcessedNotify=true;PreviousPanel();}
}


void Dialog::NextPanel(DialogPanel* thePanel)
{
	gAppPtr->UnfocusKeyboard(gAppPtr->mFocusKeyboardCPU);
	mMovePanel=-1;
	mNextPanel=thePanel;

	mCurrentPanel->mWidth-=mScrollSpeed;
	mCurrentPanel->mDrawOffset.mX-=mScrollSpeed;

	mNextPanel->mX=mPanelArea.UpperRight().mX;
	mNextPanel->mWidth=0;
	mNextPanel->Hide(false);

	mCurrentPanel->Unfocus();
	mNextPanel->Focus();

	*this+=mNextPanel;
}

void Dialog::PreviousPanel()
{
	gAppPtr->UnfocusKeyboard(gAppPtr->mFocusKeyboardCPU);
	mCurrentPanel->Unfocus();

	if (mPanelStack.IsEmpty()) 
	{
		Notify(&mDone);
		return;
	}


	mMovePanel=1;
	mNextPanel=mCurrentPanel;
	mCurrentPanel->mX+=mScrollSpeed;
	mCurrentPanel->mWidth-=mScrollSpeed;

	mCurrentPanel=mPanelStack.Pop();
	mCurrentPanel->Hide(false);
	mCurrentPanel->BindSpinsV();

	mCurrentPanel->Focus();

	if (mPanelStack.IsEmpty()) mBack.Hide();
	*mBack.mIsDown=false;
}

void Dialog::MakeCurrent()
{
	gCurrentDialog=this;

}


void Dialog::Core_Draw()
{
	if (mFeatures&CPU_DRAW)
	{
		MakeCurrent();

		if (mFeatures&CPU_CLIP)
		{
			gG.PushClip();
			gG.Clip(mX,mY,mWidth,mHeight);
		}
		gG.PushTranslate();
		gG.PushColor();

		if (mFeatures&CPU_CENTERED) gG.Translate(Center());
		else gG.Translate(UpperLeft());

		DISPATCHPROCESS(mForwardDraw,Draw());
		if (mCPUManager) mCPUManager->Draw();	// Draw all child things... notice we did Freezeclip, this is so we can have more than one panel and slide them around
		DISPATCHPROCESS(mForwardDraw,DrawOverlay());
		DrawTitle();

		gG.PopColor();
		gG.PopTranslate();
		if (mFeatures&CPU_CLIP) gG.PopClip();
	}
}

void Dialog::DrawText(Font* theFont, String theText, Point thePos)
{
	theFont->Draw(theText,thePos+Point(0.0f,theFont->mAscent));
}


DialogPanel* Dialog::MakeNewPanel(String theName, bool mainPanel)
{
	if (mPanelArea.mWidth==-1) mPanelArea=Rect(10,10,mWidth-20,mHeight-20);

	DialogPanel* aPanel=new DialogPanel;
	aPanel->mParent=this;
	aPanel->Size(mPanelArea);
	aPanel->mName=theName;

	mPanelList+=aPanel;

	//
	// If it's a sub-panel, it needs to go to the side, remember, it'll fold in.
	//
	*this+=aPanel;
	aPanel->Hide(true);
	if (mainPanel)
	{
		aPanel->Hide(false);
		mCurrentPanel=aPanel;
		mCurrentPanel->BindSpinsV();
	}
	return aPanel;
}

DialogPanel::DialogPanel()
{
	mParent=NULL;
	mNoDraw=false;
	mFirstFocus=true;
}

DialogPanel::~DialogPanel()
{
//	if (mParent) mParent->mPanelList-=this;
}

void DialogPanel::Focus()
{
#ifdef _MAGICCURSOR
	if (mParent->mDisableMagicCursor) {mMCursor.Enable(false);return;}
	if (mFirstFocus || mMCursor.mCurrentItem==mMCursor.mEscItem)
	{
		mMCursor.SetCaret(0);
	}

	//if (mParent->mPanelStack.IsEmpty()) mMCursor.AddEscButton(&mParent->mDone);
	//else mMCursor.AddEscButton(&mParent->mBack);

	MagicCursor::MagicItem* aItem=mMCursor.AddEscButton(&mParent->mBack);
	aItem->mFlag|=MCFLAG_KLUDGE;
	mMCursor.Focus();
	mMCursor.mPushed=false;
#endif
	mFirstFocus=false;
}

void DialogPanel::Unfocus()
{
#ifdef _MAGICCURSOR
	if (mParent->mDisableMagicCursor) {mMCursor.Enable(false);return;}
	RebootMagicCursor(0);
#endif
}

void DialogPanel::Core_Draw()
{
	SwipeBox::Core_Draw();
	return;

/*
	static Array<Rect> aHoldRect;
	//
	// Resize all widgets.  For touching, we make them full sized.  For drawing, we shrink them by the border we requested.
	//
	if (mCPUManager) {int aRCount=0;EnumList(DialogWidget,aCPU,*mCPUManager) if (aCPU->IsGrouped()) {aHoldRect[aRCount++]=*aCPU;aCPU->Size(aCPU->ExpandH(-CurrentWidgetBorder().mX).ExpandV(-CurrentWidgetBorder().mY));}}
	SwipeBox::Core_Draw();
	//
	// Restore the widgets
	//
	if (mCPUManager) {int aRCount=0;EnumList(DialogWidget,aCPU,*mCPUManager) if (aCPU->IsGrouped()) aCPU->Size(aHoldRect[aRCount++]);}
*/
}

DialogWidget* DialogPanel::Add(DialogWidget* theWidget)
{
	mParent->MakeCurrent();

	theWidget->mParent=this;
	theWidget->mFirstUpdate=false;
	theWidget->mWidth=mWidth-(CurrentWidgetBorder().mX*2);
	theWidget->FixAttributes();
	theWidget->Initialize();
	//theWidget->mHeight+=(CurrentWidgetBorder().mY*2);
	mWidgetList+=theWidget;

	Point aMoveChildren;
	Rect aRect=theWidget->ExpandH(CurrentWidgetBorder().mX);
	aMoveChildren.mX=CurrentWidgetBorder().mY;
	if (theWidget->IsGrouped()) 
	{
		aRect=aRect.ExpandV(CurrentWidgetBorder().mY);
		aMoveChildren.mY=CurrentWidgetBorder().mY;
	}

	if (theWidget->mCPUManager)
	{
		EnumList(CPU,aCPU,*theWidget->mCPUManager) 
		{
			aCPU->mX+=aMoveChildren.mX;
			aCPU->mY+=aMoveChildren.mY;
		}
	}

	theWidget->mX=aRect.mX;
	theWidget->mY=aRect.mY;
	theWidget->mWidth=aRect.mWidth;
	theWidget->mHeight=aRect.mHeight;

	Rehup();	// Rehupping each time seems slow, but it's not that bad.

#ifdef _MAGICCURSOR
	if (!mParent->mDisableMagicCursor) 
	{
		switch (theWidget->mID)
		{
		case DWIDGET_CHECKBOX:
		case DWIDGET_LIST:
		case DWIDGET_BUTTON:
		case DWIDGET_NEXTPANEL:
		case DWIDGET_NUMBERBOX:
		case DWIDGET_KEYGRABBER:
		case DWIDGET_JOYSTICKGRABBER:
			mMCursor.AddButton(theWidget);
			break;
		case DWIDGET_TEXTBOX:
			{
				DialogTextbox* aTB=(DialogTextbox*)theWidget;
				mMCursor.AddTextBox(&aTB->mTextBox);
				aTB->mTextBox.mExtraData=aTB;
			}
			break;
		case DWIDGET_SLIDER:
			{
				DialogSlider* aSL=(DialogSlider*)theWidget;
				mMCursor.AddSlider(&aSL->mSlider);
				aSL->mSlider.mExtraData=aSL;
			}
			break;
		}
	}
#endif

	return theWidget;
}

void DialogPanel::Reset()
{
	EnumList(DialogWidget,aDW,mWidgetList) aDW->Kill();
	mCPUManager->Update();
	mCPUManager->Update();
	mWidgetList.Clear();
	Rehup();
}

void DialogPanel::Rehup()
{
	if (mCPUManager) mCPUManager->Clear();

	mGroupRect.Reset();

	mCursor=Point(0.0f,mParent->mVPadding);

	float aCurrentHeight=0;
	EnumList(DialogWidget,aDW,mWidgetList)
	{
		aDW->FixAttributes();

		float aYBorder=mParent->mWidgetBorder.mY*aDW->IsGrouped();
		aDW->mFlag=0;
		aDW->mX=mCursor.mX;
		aDW->mY=mCursor.mY;
		*this+=aDW;
		mCursor.mX+=aDW->Width();//+(mParent->mWidgetBorder.mX*2);
		aCurrentHeight=_max(aCurrentHeight,aDW->Height());//+(aYBorder*2));

		if (mCursor.mX>=mWidth)
		{
			mCursor.mX=0;
			mCursor.mY+=aCurrentHeight+1;
			aCurrentHeight=0;
		}

//		if (aDW->IsGrouped()) mGroupRect.Add(Rect(aDW->mX,aDW->mY,aDW->mWidth+(mParent->mWidgetBorder.mX*2),aDW->mHeight+(aYBorder*2)+1));
		if (aDW->IsGrouped()) mGroupRect.Add(Rect(aDW->mX,aDW->mY,aDW->mWidth,aDW->mHeight+1));
	}

	mGroupRect.Consolidate();
	EnumList(Rect,aR,mGroupRect.mRectList) aR->mHeight-=1;

	EnumList(DialogWidget,aDW,mWidgetList)
	{
//		aDW->mY+=mParent->mWidgetBorder.mY;
		if (aDW->IsGrouped())
		{
			//aDW->mX+=mParent->mWidgetBorder.mX;
			Point aTest=aDW->Bottom().Mid();
			if (mGroupRect.ContainsPoint(aTest.mX,aTest.mY)) aDW->mFlag|=DIALOGFLAG_DRAWLINE;
		}
	}

	SetSwipeHeight(mCursor.mY+mParent->mVPadding);
}

void DialogPanel::Draw()
{
	if (mNoDraw) return;
/*
	gG.SetColor(1,0,0,.25f);
	gG.FillRect(0,0,9999,9999);
	gG.SetColor();
*/

	EnumList(Rect,aR,mGroupRect.mRectList) CurrentBundle().mGroupbox.DrawMagicBorder(*aR,true);
	
	gG.PushTranslate();
	gG.Translate(GetPos());
	EnumList(DialogWidget,aDW,mWidgetList) if (aDW->mFlag&DIALOGFLAG_DRAWLINE) 
	{
		Rect aRect=*aDW;//aDW->ExpandH(mParent->mWidgetBorder.mX*2).ExpandV(mParent->mWidgetBorder.mY);
		CurrentBundle().mGroupboxDivider.DrawMagicBorderH(aRect.mX,aRect.mY+aRect.mHeight-CurrentBundle().mGroupboxDivider.HalfHeightF(),aRect.mWidth);
		//mParent->DrawDivider(aRect);
	}
	gG.PopTranslate();

#ifdef _MAGICCURSOR
	if (!mParent->mDisableMagicCursor && !mParent->mMovePanel && mMCursor.mEnabled) DrawMagicCursor();
#endif
}

void DialogStatic::Initialize()
{
	mName=CurrentDialogFont()->Wrap(mName,mWidth);
	Rect aDims=CurrentDialogFont()->GetBoundingRect(mName,Point(0,0));
	mHeight=aDims.Height();
	if (mSprite) mHeight=_max(mHeight,mSprite->mHeight);
	DisableTouch();
}

void DialogStatic::Draw()
{
	if (mSprite) 
	{
		mSprite->Draw(0,0);
		gG.PushTranslate();
		gG.Translate(mSprite->mWidth+CurrentBundle().mWidgetBorder.mX,0.0f);
		gG.Translate(0.0f,(mHeight/2)-(CurrentDialogFont()->mAscent)/2);
		gG.SetColor(CurrentDialogFontColor());
		CurrentDialog()->DrawText(CurrentDialogFont(),mName);
		gG.SetColor();
		gG.PopTranslate();
	}
	else
	{
		gG.SetColor(CurrentDialogFontColor());
		CurrentDialog()->DrawText(CurrentDialogFont(),mName);
		gG.SetColor();
	}
}

void DialogCheckbox::Initialize()
{
	mName=CurrentDialogFont()->Wrap(mName,mWidth-(CurrentBundle().mCheckbox[0].mWidth+CurrentWidgetBorder().mX));
	Rect aDims=CurrentDialogFont()->GetBoundingRect(mName,Point(0,0));
	mHeight=aDims.Height();
	mCursor=CURSOR_FINGER;
}

void DialogCheckbox::Draw()
{
	gG.SetColor(CurrentDialogFontColor());
	CurrentDialog()->DrawText(CurrentDialogFont(),mName);
	gG.SetColor();

	CurrentBundle().mCheckbox[*mHook].Center(mWidth-CurrentBundle().mCheckbox[*mHook].HalfWidthF(),mHeight/2);
}

void DialogCheckboxInverted::Draw()
{
	gG.SetColor(CurrentDialogFontColor());
	CurrentDialog()->DrawText(CurrentDialogFont(),mName);
	gG.SetColor();

	CurrentBundle().mCheckbox[1-(*mHook)].Center(mWidth-CurrentBundle().mCheckbox[*mHook].HalfWidthF(),mHeight/2);
}

void DialogCheckbox::TouchEnd(int x, int y)
{
	DialogWidget::TouchEnd(x,y);
	if (x>0 && y>0 && x<mWidth && y<mHeight)
	{
		MakeCurrent();
		CurrentBundle().mClickSound->Play();
		bool aWhat=*mHook;
		*mHook=!aWhat;

		CurrentDialog()->ChangedControl(this);
	}
}

void DialogTextbox::Initialize()
{
	Rect aDims=CurrentDialogFont()->GetBoundingRect(mName,Point(0,0));
	mHeight=aDims.Height();

	mTextBox.Size(aDims.mWidth+CurrentWidgetBorder().mX,0,mWidth-(aDims.mWidth+CurrentWidgetBorder().mX),CurrentDialogFont()->mAscent+6);
	mTextBox.SetText(*mHook);
	mTextBox.mY=HalfHeight()-mTextBox.HalfHeight();
	mTextBox.SetBackgroundMagicSprite(&CurrentBundle().mTextboxBKG);
	mTextBox.SetFont(CurrentDialogFont());
	mTextBox.SetTextColor(CurrentBundle().mTextBoxColor);
	mTextBox.SetBorder((int)CurrentBundle().mTextBoxBorder.mX,(int)CurrentBundle().mTextBoxBorder.mY);
	mTextBox.SetTextOffset(CurrentBundle().mTextBoxTextOffset);
	mTextBox.SetCursorOffset(CurrentBundle().mTextBoxCursorOffset);
	mTextBox.SetCursorWidth(CurrentBundle().mTextBoxCursorWidth);
	mTextBox.SetNotify(this);
	mTextBox.mY=(aDims.mHeight/2)-(mTextBox.mHeight/2);
	mTextBox.mX+=CurrentBundle().mTextBoxOffset.mX;
	mTextBox.mY+=CurrentBundle().mTextBoxOffset.mY;
	*this+=&mTextBox;

	//aTB->mTextBox.SetBorder(8,5);
	//aTB->mTextBox.SetCursorOffset(Point(2,2));
	//aTB->mTextBox.SetCursorWidth(3);

}

void DialogTextbox::Draw()
{
	gG.SetColor(CurrentDialogFontColor());
	CurrentDialog()->DrawText(CurrentDialogFont(),mName);
	gG.SetColor();
}

void DialogTextbox::Notify(void* theData)
{
	if (theData==&mTextBox) 
	{
		MakeCurrent();
		*mHook=mTextBox.GetText();
		CurrentDialog()->ChangedControl(this);
	}
}

void DialogSlider::Initialize()
{
	Rect aDims=CurrentDialogFont()->GetBoundingRect(mName,Point(0,0));
	mHeight=aDims.Height();

	mSlider.mThumb.SetSprites(CurrentBundle().mSliderThumb);
	mSlider.SetSprite(CurrentBundle().mSliderBKG,true);
	mSlider.Size(aDims.mWidth+CurrentWidgetBorder().mX,0,mWidth-(aDims.mWidth+CurrentWidgetBorder().mX));
	mSlider.mY=(aDims.mHeight/2)-(mSlider.mHeight/2);
	mSlider.SetNotify(this);
	mSlider.SetValue(*mHook);
	mSlider.SetBorder(CurrentBundle().mSliderThumb[0].HalfWidthF()+3);
	*this+=&mSlider;
}

void DialogSlider::Draw()
{
	gG.SetColor(CurrentDialogFontColor());
	CurrentDialog()->DrawText(CurrentDialogFont(),mName);
	gG.SetColor();
}

void DialogSlider::Notify(void* theData)
{
	if (theData==&mSlider)
	{
		MakeCurrent();
		*mHook=mSlider.GetValue();
		CurrentDialog()->ChangedControl(this);
	}
}



void DialogWidget::FixAttributes()
{
	if (!mFont) mFont=CurrentDialogFont();
	if (mColor.mA==-1) mColor=CurrentDialogFontColor();
}

//
// Complicated!  Widgets have to take into account their border size when drawing, but NOT when mousing.  But, we also don't want the children widgets to take the border size into
// account, since they have been moved previously.
//
void DialogWidget::Core_Draw()
{
	if (!IsGrouped()) {CPU::Core_Draw();return;}

	if (mFeatures&CPU_DRAW)
	{
		if (mFeatures&CPU_CLIP)
		{
			gG.PushClip();
			gG.Clip(mX,mY,mWidth,mHeight);
		}
		gG.PushTranslate();
		gG.PushColor();

		if (IsGrouped())
		{
			mX+=mParent->mParent->mWidgetBorder.mX;
			mY+=mParent->mParent->mWidgetBorder.mY;
			mWidth-=mParent->mParent->mWidgetBorder.mX*2;
			mHeight-=mParent->mParent->mWidgetBorder.mY*2;
		}
		gG.PushTranslate();
		if (mFeatures&CPU_CENTERED) gG.Translate(Center());
		else gG.Translate(UpperLeft());
		DISPATCHPROCESS(mForwardDraw,Draw());
		gG.PopTranslate();
		if (IsGrouped())
		{
			mX-=mParent->mParent->mWidgetBorder.mX;
			mY-=mParent->mParent->mWidgetBorder.mY;
			mWidth+=mParent->mParent->mWidgetBorder.mX*2;
			mHeight+=mParent->mParent->mWidgetBorder.mY*2;
		}

		gG.PushTranslate();
		if (mFeatures&CPU_CENTERED) gG.Translate(Center());
		else gG.Translate(UpperLeft());
		if (mCPUManager) mCPUManager->Draw();
		gG.PopTranslate();

		if (IsGrouped())
		{
			mX+=mParent->mParent->mWidgetBorder.mX;
			mY+=mParent->mParent->mWidgetBorder.mY;
			mWidth-=mParent->mParent->mWidgetBorder.mX*2;
			mHeight-=mParent->mParent->mWidgetBorder.mY*2;
		}
		gG.PushTranslate();
		if (mFeatures&CPU_CENTERED) gG.Translate(Center());
		else gG.Translate(UpperLeft());
		DISPATCHPROCESS(mForwardDraw,DrawOverlay());
		gG.PopTranslate();
		if (IsGrouped())
		{
			mX-=mParent->mParent->mWidgetBorder.mX;
			mY-=mParent->mParent->mWidgetBorder.mY;
			mWidth+=mParent->mParent->mWidgetBorder.mX*2;
			mHeight+=mParent->mParent->mWidgetBorder.mY*2;
		}

		gG.PopColor();
		gG.PopTranslate();
		if (mFeatures&CPU_CLIP) gG.PopClip();
	}
}


//
// Dialogwidget needs a special hittest, since we're resizing them all the time
//
CPU* DialogWidget::HitTest(float x, float y)
{
	return CPU::HitTest(x,y);
	//Twixfix

/*
	if (ContainsPoint(x,y)) 
	{
		if (mCPUManager) 
		{
			Point aWork=Point(x,y);
			if (mFeatures&CPU_CENTERED) aWork-=Center();
			else aWork-=UpperLeft();

			aWork.mX-=mParent->mParent->mWidgetBorder.mX;
			aWork.mY-=mParent->mParent->mWidgetBorder.mY;
			CPU *aSubCPU=mCPUManager->Pick(aWork.mX,aWork.mY);
			if (aSubCPU) return aSubCPU;
		}
		return this;
	}
	return NULL;
*/
}

void DialogWidget::TouchStart(int x, int y)
{
	mIsDown=true;
}

void DialogWidget::TouchMove(int x, int y)
{
	if (mParent->CanScroll()) if (mIsDown) if (gMath.Abs(gAppPtr->mMessageData_TouchPosition.mY-gAppPtr->mMessageData_TouchStartPosition.mY)>gDialogDragMargin)
	{
		mIsDown=false;
		Point aHoldPos=gAppPtr->mMessageData_TouchStartPosition;
		gAppPtr->mMessageData_TouchPosition=Point(-9999,-9999);
		gAppPtr->ForceTouchEnd(true);
		int aHold=mFeatures;
		gAppPtr->mMessageData_TouchPosition=aHoldPos;
		DisableFeature(CPU_TOUCH);
		gAppPtr->ForceTouchStart();
		mFeatures=aHold;
	}
}

void DialogWidget::TouchEnd(int x, int y)
{
	mIsDown=false;
}

void DialogList::Initialize()
{
	mName=CurrentDialogFont()->Wrap(mName,mWidth/2);
	Rect aDims=CurrentDialogFont()->GetBoundingRect(mName,Point(0,0));
	mHeight=aDims.Height();
	mCursor=CURSOR_FINGER;

	//
	// Need to make a panel...
	//
	mPanel=CurrentDialog()->MakeNewPanel(mName,false);
	mPanel->mName=mPanel->mName.RemoveTail(':');
	mPanel->mName=mPanel->mName.RemoveTail('.');
	for (int aCount=0;aCount<mChoiceList.Size();aCount++)
	{
		DialogWidget* aWidget=mPanel->Add(new DialogButton(mChoiceList[aCount].mText,false,this));
		aWidget->ExtraData()=(void*)mChoiceList[aCount].mValue;

		mChoiceListRef[mChoiceList[aCount].mValue]=aCount;
	}
	*mHook=_clamp(0,*mHook,mChoiceListRef.Size()-1);
}

void DialogList::Draw()
{
	gG.SetColor(CurrentDialogFontColor());
	CurrentDialog()->DrawText(CurrentDialogFont(),mName);
	gG.SetColor(CurrentDialogClickableColor());
    
    if (mChoiceList.Size())
    {
        String& aStr=mChoiceList[mChoiceListRef[*mHook]].mText;

        //for (int aCount=0;aCount<mChoiceList.Size();aCount++) gOut.Out("Choice: %s",mChoiceList[aCount].mText.c());
        //for (int aCount=0;aCount<mChoiceListRef.Size();aCount++) gOut.Out("Ref: %d",mChoiceListRef[aCount]);
	
        CurrentDialog()->DrawText(CurrentDialogFont(),aStr,Point(mWidth-CurrentDialogFont()->Width(aStr)-CurrentBundle().mChoiceArrow.WidthF(),0.0f));
        CurrentBundle().mChoiceArrow.Center(mWidth-CurrentBundle().mChoiceArrow.HalfWidthF(),HalfHeight());
    }
	gG.SetColor();
}

void DialogList::Notify(void* theData)
{
	DialogWidget* aWidget=(DialogWidget*)theData;
	int aValue=(uintptr_t)aWidget->ExtraData();

	MakeCurrent();
	*mHook=_clamp(0,aValue,mChoiceListRef.Size()-1);
	CurrentDialog()->ChangedControl(this);
	mParent->mParent->PreviousPanel();
}


void DialogList::TouchEnd(int x, int y)
{
	DialogWidget::TouchEnd(x,y);
	if (x>0 && y>0 && x<mWidth && y<mHeight)
	{
		MakeCurrent();
		CurrentBundle().mClickSound->Play();
		CurrentDialog()->NextPanel(mPanel);
	}
}

void DialogButton::Initialize()
{
	mName=CurrentDialogFont()->Wrap(mName,mWidth);
	Rect aDims=CurrentDialogFont()->GetBoundingRect(mName,Point(0,0));
	mHeight=aDims.Height();
	mCursor=CURSOR_FINGER;
}

void DialogButton::Draw()
{
/*
	float aX=0;
	if (mIcon) 
	{
		CurrentBundle().mTouchIcon.Center(CurrentBundle().mTouchIcon.HalfWidthF(),mHeight/2.0f);
		aX+=CurrentBundle().mTouchIcon.WidthF();
	}
	gG.SetColor(CurrentDialogFontColor());
	CurrentDialog()->DrawText(CurrentDialogFont(),mName,Point(aX,0.0f));
	gG.SetColor();
*/

	gG.SetColor(mColor);//CurrentDialogFontColor());
	CurrentDialog()->DrawText(CurrentDialogFont(),mName,mTextOffset);
	if (mIcon) CurrentBundle().mTouchIcon.Center(mWidth-CurrentBundle().mTouchIcon.HalfWidthF(),mHeight/2.0f);
	gG.SetColor();
}

void DialogButton::TouchStart(int x, int y)
{
	DialogWidget::TouchStart(x,y);
	MakeCurrent();
	CurrentBundle().mClickSound->Play();
}

void DialogButton::TouchEnd(int x, int y)
{
	DialogWidget::TouchEnd(x,y);
	if (x>0 && y>0 && x<mWidth && y<mHeight)
	{
		if (mNotify) mNotify->Notify(this);
		else 
		{
			CurrentDialog()->ClickControl(this);
			CurrentDialog()->ChangedControl(this);
		}
	}
}

void DialogKeyGrabber::Initialize()
{
	mName=CurrentDialogFont()->Wrap(mName,mWidth);
	Rect aDims=CurrentDialogFont()->GetBoundingRect(mName,Point(0,0));
	mHeight=aDims.Height();
	mCursor=CURSOR_FINGER;
}

void DialogKeyGrabber::Draw()
{
	gG.SetColor(CurrentDialogFontColor());
	CurrentDialog()->DrawText(CurrentDialogFont(),mName);
	gG.SetColor();

	CurrentBundle().mKeygrabberBKG.DrawTelescopedH(mWidth-(mWidth/3),0,(mWidth/3));
	gG.SetColor(CurrentBundle().mKeyGrabberColor);
	gG.PushTranslate();
	gG.Translate(CurrentBundle().mKeyGrabberTextOffset);
	if (mGrabbing)
	{
		if ((gAppPtr->AppTime()/20)%2) CurrentBundle().mFont_GroupHeader.Center("PRESS KEY",mWidth-(mWidth/6),(CurrentBundle().mFont_GroupHeader.mAscent/2)+(mHeight/2));
	}
	else CurrentBundle().mFont_GroupHeader.Center(gInput.GetKeyName(*mHook),mWidth-(mWidth/6),(CurrentBundle().mFont_GroupHeader.mAscent/2)+(mHeight/2));
	gG.PopTranslate();
	gG.SetColor();
}

void DialogKeyGrabber::TouchStart(int x, int y)
{
	DialogWidget::TouchStart(x,y);
	MakeCurrent();
	CurrentBundle().mClickSound->Play();
}

void DialogKeyGrabber::TouchEnd(int x, int y)
{
	DialogWidget::TouchEnd(x,y);
	if (x>0 && y>0 && x<mWidth && y<mHeight)
	{
		CurrentDialog()->ClickControl(this);
		mGrabbing=true;
#ifdef _MAGICCURSOR
		mParent->mMCursor.Enable(false);
#endif
		KeyGrabber* aKG=new KeyGrabber();
		aKG->ExtraData()=this;
#ifndef _MAGICCURSOR
		aKG->Go(CPUHOOK(
			{
				DialogKeyGrabber* aKG=(DialogKeyGrabber*)theThis->ExtraData();
				*aKG->mHook=theThis->mThrottleResult;
				aKG->mGrabbing=false;
				CurrentBundle().mClickSound->Play();
				CurrentDialog()->ChangedControl(this);
			}
		));
#else
		aKG->Go(CPUHOOK(
			{
				DialogKeyGrabber* aKG=(DialogKeyGrabber*)theThis->mExtraData;
				*aKG->mHook=theThis->mThrottleResult;
				aKG->mParent->mMCursor.Enable(true);
				aKG->mGrabbing=false;
				CurrentBundle().mClickSound->Play();
				CurrentDialog()->ChangedControl(this);
			}
		));

#endif
		/*
		if (aResult>0) *mHook=aResult;
		#ifdef _MAGICCURSOR
		mParent->mMCursor.Enable(true);
		#endif
		mGrabbing=false;
		CurrentBundle().mClickSound->Play();
		CurrentDialog()->ChangedControl(this);
		*/
	}
}

void Dialog::DrawTitle()
{
	if (mTiny) return;
	if (mCurrentPanel)
	{
		gG.PushTranslate();
		gG.Translate(CurrentBundle().mHeaderTextOffset);
		float aY=(mPanelArea.mY/2)+mTitleFont->mAscent/2;

		float aFade=1.0f;
		if (mMovePanel) aFade=mMovePanelProgress;
		gG.SetColor(mTitleFontColor,mTitleFontColor.mA*aFade*aFade);
		mTitleFont->Center(mCurrentPanel->mName,mWidth/2-((1.0f-aFade)*mPanelArea.mWidth/3),aY);
		gG.SetColor();

		if (mMovePanel)
		{
			aFade=1.0f-(mMovePanelProgress);
			gG.SetColor(mTitleFontColor,mTitleFontColor.mA*aFade*aFade);
			mTitleFont->Center(mNextPanel->mName,mWidth/2+((1.0f-aFade)*mPanelArea.mWidth/3),aY);
			gG.SetColor();
		}
		gG.PopTranslate();
	}
}

void DialogGroupHeader::Initialize()
{
	if (mColor.mA<0) mColor=CurrentBundle().mGroupHeaderColor;
	mFont=&CurrentBundle().mFont_GroupHeader;
	mName=mFont->Wrap(mName,mWidth);
	Rect aDims=mFont->GetBoundingRect(mName,Point(0,0));
	mHeight=aDims.Height();
}

void DialogGroupHeader::Draw()
{
	gG.SetColor(mColor);
	CurrentDialog()->DrawText(mFont,mName,Point(CurrentDialog()->mWidgetBorder.mX,0.0f));
	gG.SetColor();
}

void Dialog::Draw()
{
	if (mBundle->mBackgroundIs4Corner) mBundle->mBKGCorner.DrawMagicBorderEX_AllCorners(Rect(0,0,mWidth,mHeight),true);
	else mBundle->mBKGCorner.DrawMagicBorder(Rect(0,0,mWidth,mHeight),true);
}

void Dialog::SetBundle(DialogWidgetBundle* theBundle)
{
	mBundle=theBundle;
	mBundle->Load();

	SetPanelArea(mBundle->mPanelArea.mX1,mBundle->mPanelArea.mY1,mWidth+mBundle->mPanelArea.mX2,mHeight+mBundle->mPanelArea.mY2);
	SetTitleFont(&theBundle->mFont_Header);
	SetFont(&theBundle->mFont_Normal);
	SetTitleFontColor(theBundle->mTitleColor);		// Get from Bundle
	SetFontColor(theBundle->mNormalColor);		// Get from Bundle
	SetClickableColor(theBundle->mClickableColor);	// Get from Bundle
	SetWidgetBorder(theBundle->mWidgetBorder);	// Get From Bundle
	SetVPadding(theBundle->mVerticalPadding);	// Get From Bundle
}

void Dialog::DrawOverlay()
{
	if (!mTiny)
	{
		gG.FillRectGradientV(8,mPanelArea.mY,mWidth-(8*2),10,Color(0,0,0,.5f),Color(0,0,0,0));
		gG.FillRectGradientV(8,mPanelArea.mY+mPanelArea.mHeight-10,mWidth-(8*2),10,Color(0,0,0,0),Color(0,0,0,.5f));
	}
}

void Dialog::SetupButtons()
{
	if (mTiny) return;

	Point aCenter=mDone.Center();
	mDone.SetSprites(mBundle->mDoneButton);
	mDone.CenterAt(aCenter+Point(0,-5)+mBundle->mDoneButtonOffset);
	mDone.SetSounds(mBundle->mClickSound);

	mBack.SetSprites(mBundle->mBackButton);
	mBack.mX+=25+mBundle->mBackButtonOffset.mX;
	mBack.mY+=25+mBundle->mBackButtonOffset.mY;
	mBack.Size(mBack.Expand(10));
	mBack.SetSounds(mBundle->mClickSound);
}

void DialogNextPanel::Initialize()
{
	mName=CurrentDialogFont()->Wrap(mName,mWidth);
	Rect aDims=CurrentDialogFont()->GetBoundingRect(mName,Point(0,0));
	mHeight=aDims.Height();
	mCursor=CURSOR_FINGER;
}

void DialogNextPanel::Draw()
{
	gG.SetColor(CurrentDialogFontColor());
	CurrentDialog()->DrawText(CurrentDialogFont(),mName);

	gG.SetColor(CurrentDialogClickableColor());
	CurrentDialog()->DrawText(CurrentDialogFont(),mPanelText,Point(mWidth-CurrentDialogFont()->Width(mPanelText)-CurrentBundle().mChoiceArrow.WidthF(),0.0f));
	CurrentBundle().mChoiceArrow.Center(mWidth-CurrentBundle().mChoiceArrow.HalfWidthF(),HalfHeight());
	gG.SetColor();
}

void DialogNextPanel::TouchStart(int x, int y)
{
	DialogWidget::TouchStart(x,y);
	MakeCurrent();
	CurrentBundle().mClickSound->Play();
}

void DialogNextPanel::TouchEnd(int x, int y)
{
	DialogWidget::TouchEnd(x,y);
	if (x>0 && y>0 && x<mWidth && y<mHeight) mParent->mParent->NextPanel(mPanel);
}


void DialogNumberbox::Initialize()
{
	Rect aDims=CurrentDialogFont()->GetBoundingRect(mName,Point(0,0));
	mHeight=aDims.Height();

	float aTweakSpace=2;
	float aButtonSpace=CurrentBundle().mTweak[0].WidthF()+aTweakSpace;

	float aMaxWidth=(CurrentDialogFont()->Width(mFitNumber));
	aMaxWidth+=(int)CurrentBundle().mTextBoxBorder.mX*2;	// Text Border
	float aTotalWidth=aMaxWidth;
	aTotalWidth+=(aButtonSpace*2);	// Tweak buttons plus spacers...

	float aMinX=aDims.mWidth+CurrentWidgetBorder().mX;		// Minimum X position we can have without overwriting text...
	float aWantX=mWidth-CurrentWidgetBorder().mX-aTotalWidth;						// The X we want...
	if (aWantX<aMinX)
	{
		float aDiff=aMinX-aWantX;
		aMaxWidth-=aDiff;
		aWantX=aMinX;
	}

	//mTextBox.Size(aDims.mWidth+CurrentWidgetBorder().mX,0,mWidth-(aDims.mWidth+CurrentWidgetBorder().mX),CurrentDialogFont()->mAscent+6);
	mTextBox.Size(aWantX+aButtonSpace,0,aMaxWidth,CurrentDialogFont()->mAscent+6);
	mTextBox.SetText(Sprintf("%d",*mHook));
	mTextBox.mY=HalfHeight()-mTextBox.HalfHeight();
	mTextBox.mHasX=false;
	mTextBox.SetBackgroundMagicSprite(&CurrentBundle().mTextboxBKG);
	mTextBox.SetFont(CurrentDialogFont());
	mTextBox.SetTextColor(CurrentBundle().mTextBoxColor);
	mTextBox.SetBorder((int)CurrentBundle().mTextBoxBorder.mX,(int)CurrentBundle().mTextBoxBorder.mY);
	mTextBox.SetTextOffset(CurrentBundle().mTextBoxTextOffset);
	mTextBox.SetCursorOffset(CurrentBundle().mTextBoxCursorOffset);
	mTextBox.SetCursorWidth(CurrentBundle().mTextBoxCursorWidth);
	mTextBox.SetNotify(this);
	*this+=&mTextBox;

	mTweakUp.Size(mTextBox.UpperRight().mX+aTweakSpace,0,CurrentBundle().mTweak[0].WidthF(),CurrentBundle().mTweak[0].HeightF());
	mTweakUp.CenterAt(mTweakUp.CenterX(),mTextBox.CenterY());
	mTweakUp.SetNotify(this);
	mTweakUp.SetSprites(CurrentBundle().mTweak[0],CurrentBundle().mTweak[0]);
	mTweakUp.NotifyOnPress();
	mTweakUp.SetSounds(CurrentBundle().mClickSound);
	*this+=&mTweakUp;

	mTweakDown.Size(mTextBox.mX-aButtonSpace,0,CurrentBundle().mTweak[0].WidthF(),CurrentBundle().mTweak[0].HeightF());
	mTweakDown.CenterAt(mTweakDown.CenterX(),mTextBox.CenterY());
	mTweakDown.SetNotify(this);
	mTweakDown.SetSprites(CurrentBundle().mTweak[1],CurrentBundle().mTweak[1]);
	mTweakDown.NotifyOnPress();
	mTweakDown.SetSounds(CurrentBundle().mClickSound);
	*this+=&mTweakDown;


	//aTB->mTextBox.SetBorder(8,5);
	//aTB->mTextBox.SetCursorOffset(Point(2,2));
	//aTB->mTextBox.SetCursorWidth(3);

}

void DialogNumberbox::Draw()
{
	gG.SetColor(CurrentDialogFontColor());
	CurrentDialog()->DrawText(CurrentDialogFont(),mName);
	gG.SetColor();
}

void DialogNumberbox::Notify(void* theData)
{
	if (theData==&mTextBox) 
	{
		MakeCurrent();
		*mHook=_clamp(mMin,mTextBox.GetText().ToInt(),mMax);
		mTextBox.SetText(Sprintf("%d",*mHook));
		CurrentDialog()->ChangedControl(this);
	}
	if (theData==&mTweakUp)
	{
		MakeCurrent();
		int aValue=*mHook;
		aValue++;
		*mHook=_clamp(mMin,aValue,mMax);
		mTextBox.SetText(Sprintf("%d",*mHook));
		CurrentDialog()->ChangedControl(this);
	}
	if (theData==&mTweakDown)
	{
		MakeCurrent();
		int aValue=*mHook;
		aValue--;
		*mHook=aValue;
		*mHook=_clamp(mMin,aValue,mMax);
		mTextBox.SetText(Sprintf("%d",*mHook));
		CurrentDialog()->ChangedControl(this);
	}
}



void DialogJoystickButtonGrabber::Initialize()
{
	mName=CurrentDialogFont()->Wrap(mName,mWidth);
	Rect aDims=CurrentDialogFont()->GetBoundingRect(mName,Point(0,0));
	mHeight=aDims.Height();
	mCursor=CURSOR_FINGER;
}

void DialogJoystickButtonGrabber::Draw()
{
	gG.SetColor(CurrentDialogFontColor());
	CurrentDialog()->DrawText(CurrentDialogFont(),mName);
	gG.SetColor();

	CurrentBundle().mKeygrabberBKG.DrawTelescopedH(mWidth-(mWidth/3),0,(mWidth/3));
	gG.SetColor(CurrentBundle().mKeyGrabberColor);
	gG.PushTranslate();
	gG.Translate(CurrentBundle().mKeyGrabberTextOffset);
	if (mGrabbing)
	{
		if ((gAppPtr->AppTime()/20)%2) CurrentBundle().mFont_GroupHeader.Center("PRESS BUTTON",mWidth-(mWidth/6),(CurrentBundle().mFont_GroupHeader.mAscent/2)+(mHeight/2));
	}
	else 
	{
		String aStr;
		//if (*mHook==JOYSTICK_TRIGGER1) aStr="Trigger 1";
		//else if (*mHook==JOYSTICK_TRIGGER2) aStr="Trigger 2";
		//else
		aStr=Sprintf("Button %d",*mHook);
		CurrentBundle().mFont_GroupHeader.Center(aStr,mWidth-(mWidth/6),(CurrentBundle().mFont_GroupHeader.mAscent/2)+(mHeight/2));
	}
	gG.PopTranslate();
	gG.SetColor();
}

void DialogJoystickButtonGrabber::TouchStart(int x, int y)
{
	DialogWidget::TouchStart(x,y);
	MakeCurrent();
	CurrentBundle().mClickSound->Play();
}

void DialogJoystickButtonGrabber::TouchEnd(int x, int y)
{
	DialogWidget::TouchEnd(x,y);
	if (x>0 && y>0 && x<mWidth && y<mHeight)
	{
		CurrentDialog()->ClickControl(this);
		//
		// Watch for button?
		//

#ifdef _MAGICCURSOR
		mParent->mMCursor.Enable(false);
#endif
		mGrabbing=true;
		JoyButtonGrabber* aKG=new JoyButtonGrabber();
		aKG->ExtraData()=this;
#ifndef _MAGICCURSOR
		aKG->Go(CPUHOOK(
			{
				DialogJoystickButtonGrabber* aKG=(DialogJoystickButtonGrabber*)theThis->ExtraData();
				*aKG->mHook=theThis->mThrottleResult;
				aKG->mGrabbing=false;
				CurrentBundle().mClickSound->Play();
				CurrentDialog()->ChangedControl(this);
			}
		));
#else
		aKG->Go(CPUHOOK(
		{
			DialogJoystickButtonGrabber* aKG=(DialogJoystickButtonGrabber*)theThis->mExtraData;
			*aKG->mHook=theThis->mThrottleResult;
			aKG->mParent->mMCursor.Enable(true);
			aKG->mGrabbing=false;
			CurrentBundle().mClickSound->Play();
			CurrentDialog()->ChangedControl(this);
		}
		));

#endif
	}
}

#ifdef _MAGICCURSOR
#include "bundle_sounds.h"
#include "bundle_ui.h"
void DialogMagicCursor::SelectChanged()
{
	if (mFlags&0x01) return;
	gSounds->mBlip.Play();
}

void DialogMagicCursor::Draw()
{
	if (!IsMagicCursorEnabled()) return;
	if (!mCurrentItem) return;

	if (mCursorBlink.GetState())
	{
		Rect aRect=GetItemRect().Expand(5);
		DialogWidget* aCPU=(DialogWidget*)mCurrentItem->mCPU;
		if (mCurrentItem->mType==(MCFLAG_TEXTBOX))
		{
			DialogTextbox* aTB=(DialogTextbox*)mCurrentItem->mCPU->mExtraData;
			aRect=aTB->ConvertRectToScreen();
			aRect=aRect.Expand(5);
			aCPU=aTB;
		}
		if (mCurrentItem->mType==(MCFLAG_SLIDER))
		{
			DialogSlider* aSL=(DialogSlider*)mCurrentItem->mCPU->mExtraData;
			aRect=aSL->ConvertRectToScreen();
			aRect=aRect.Expand(5);
			aCPU=aSL;
		}

		//gG.SetColor(0,0,0,.5f);

		//gBundle_UI->mCorner_Outline.DrawMagicBorder(aRect,false);
		//gG.RenderAdditive();

		DialogPanel* aPanel=aCPU->mParent;
		if (aPanel)
		{
			gG.PushTranslate();
			gG.PushClip();
			gG.Translate(aPanel->ConvertLocalToScreen(Point(0,0)));
			gG.PushFreezeClip();
			gG.FreezeClip(false);
			gG.Clip(aCPU->Expand(5));

			gG.SetColor(.75f,1,.75f);
			//gG.SetColor(0.0f,71.0f/256.0f,112.0f/256.0f);
			CurrentBundle().mGroupbox.DrawMagicBorder(aCPU->Expand(5),true);
	//		EnumList(Rect,aR,aPanel->mGroupRect.mRectList) CurrentBundle().mGroupbox.DrawMagicBorder(aR->Expand(5),true);
			gG.PopFreezeClip();
			gG.PopClip();
			gG.PushClip();
			gG.Clip(aCPU->ExpandDown(1));
			gG.SetColor(.5f,1,.5f);
			EnumList(Rect,aR,aPanel->mGroupRect.mRectList) CurrentBundle().mGroupbox.DrawMagicBorder(*aR,true);
			gG.PopClip();
			gG.PopTranslate();
		}

		aCPU->Draw();
		//gG.RenderNormal();

		//gG.FillRect(aRect);
		//gG.SetColor();
	}
}
#endif
