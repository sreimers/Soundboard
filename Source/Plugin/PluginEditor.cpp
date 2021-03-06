/*
 ==============================================================================

 PluginEditor.cpp
 Author:  Daniel Lindenfelser

 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

SoundboardAudioProcessorEditor::SoundboardAudioProcessorEditor(SoundboardAudioProcessor &p)
        : AudioProcessorEditor(&p), processor(p), mPauseState(true), mTimerState(true)
{
#if JUCE_WINDOWS
#if JUCE_OPENGL
    openGLContext.attachTo (*getTopLevelComponent());
#endif
#endif
    addAndMakeVisible(topBar = new Bar());
    
    addAndMakeVisible(gainSlider = new Slider());
    gainSlider->setRange(0, 100, 1);
    gainSlider->setValue(gainSlider->proportionOfLengthToValue(processor.getGain()), dontSendNotification);
    gainSlider->setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    gainSlider->setSliderStyle(Slider::SliderStyle::LinearHorizontal);
    gainSlider->addListener(this);
    gainBubble = new BubbleMessageComponent();
    gainBubble->addToDesktop(0);
    gainBubble->setAllowedPlacement(BubbleMessageComponent::BubblePlacement::below);

    addAndMakeVisible(loadDirectoryButton = new TextButton());
    loadDirectoryButton->setButtonText(FontAwesome_FolderOpenO);
    loadDirectoryButton->setLookAndFeel(awesomeLookAndFeel);
    loadDirectoryButton->setConnectedEdges(TextButton::ConnectedOnLeft | TextButton::ConnectedOnRight);
    loadDirectoryButton->addListener(this);

    addAndMakeVisible(listButton = new TextButton());
    listButton->setLookAndFeel(awesomeLookAndFeel);
    listButton->setButtonText(FontAwesome_List);
    listButton->addListener(this);
    listButton->setConnectedEdges(TextButton::ConnectedOnRight);

    addAndMakeVisible(gridButton = new TextButton());
    gridButton->setLookAndFeel(awesomeLookAndFeel);
    gridButton->setButtonText(FontAwesome_Th);
    gridButton->addListener(this);
    gridButton->setConnectedEdges(TextButton::ConnectedOnLeft);

    addAndMakeVisible(settingsButton = new TextButton());
    settingsButton->setLookAndFeel(awesomeLookAndFeel);
    settingsButton->setButtonText(FontAwesome_Cog);
    settingsButton->addListener(this);
    settingsButton->setConnectedEdges(TextButton::ConnectedOnLeft);

    addAndMakeVisible(duckButton = new TextButton());
    duckButton->setLookAndFeel(awesomeLookAndFeel);
    duckButton->setButtonText(FontAwesome_CommentO);
    duckButton->addListener(this);
    
    addAndMakeVisible(lockButton = new TextButton());
    lockButton->setLookAndFeel(awesomeLookAndFeel);
    if (processor.getLocked()) {
        lockButton->setButtonText(FontAwesome_Lock);
    } else {
        lockButton->setButtonText(FontAwesome_Unlock);
    }
    lockButton->addListener(this);
    lockButton->setConnectedEdges(TextButton::ConnectedOnRight);
    
    addAndMakeVisible(table = new SoundboardTableComponent(p));
    listButton->setEnabled(false);

    if (processor.wrapperType != AudioProcessor::wrapperType_Standalone)
    {
        addAndMakeVisible(resizableCornerComponent = new ResizableCornerComponent(this, &resizeLimits));
        resizeLimits.setSizeLimits(380, 320, 1024, 768);
    }

    refresh();

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(processor.getWindowWidth(), processor.getWindowHeight());

    startTimer(TimerIdRefresh, static_cast<int>(1000 * 0.5));
    
    // listen to gain changes
    processor.getOscManager()->addOscParameterListener(this, "/ultraschall/soundboard/gain$");
    processor.getOscManager()->addOscParameterListener(this, "/ultraschall/soundboard/duck/gain$");
    processor.getOscManager()->addOscParameterListener(this, "/ultraschall/soundboard/duck/enabled$");
}

SoundboardAudioProcessorEditor::~SoundboardAudioProcessorEditor()
{
    processor.getOscManager()->removeOscParameterListener(this);
    stopTimer(TimerIdRefresh);
    topBar                      = nullptr;
    loadDirectoryButton         = nullptr;
    table                       = nullptr;
    grid                        = nullptr;
    resizableCornerComponent    = nullptr;
    settingsButton              = nullptr;
    gridButton                  = nullptr;
    gainSlider                  = nullptr;
    gainBubble                  = nullptr;
    lockButton                  = nullptr;
}

void SoundboardAudioProcessorEditor::paint(Graphics &g)
{
    g.fillAll(ThemeBackground1);
}

void SoundboardAudioProcessorEditor::resized()
{
    topBar->setBounds(0, 0, getWidth(), 32);


    listButton->setBounds(5, 5, 60, 24);
    gridButton->setBounds(65, 5, 60, 24);

    duckButton->setBounds(260, 5, 60, 24);
    
    gainSlider->setBounds(140, 5, 100, 24);

    lockButton->setBounds(getWidth() - 185, 5, 60, 24);
    loadDirectoryButton->setBounds(getWidth() - 125, 5, 60, 24);
    settingsButton->setBounds(getWidth() - 65, 5, 60, 24);

    if (table != nullptr) {
        table->setBounds(0, 32, getWidth(), getHeight() - 32);
    }
    if (grid != nullptr) {
        grid->setBounds(0, 32, getWidth(), getHeight() - 32);
    }

    if (resizableCornerComponent)
    {
        resizableCornerComponent->setBounds(getWidth() - 16, getHeight() - 16, 16, 16);
    }

    processor.storeWindowWidth(getWidth());
    processor.storeWindowHeight(getHeight());
}

void SoundboardAudioProcessorEditor::preload() {
    showTable();
}

void SoundboardAudioProcessorEditor::showTable() {
    listButton->setEnabled(false);
    gridButton->setEnabled(true);
    
    grid = nullptr;
    addAndMakeVisible(table = new SoundboardTableComponent(processor));
    table->toFront(true);
    if (resizableCornerComponent)
    {
        resizableCornerComponent->toFront(false);
    }
    resized();
}

void SoundboardAudioProcessorEditor::showGrid() {
    gridButton->setEnabled(false);
    listButton->setEnabled(true);
    
    table = nullptr;
    addAndMakeVisible(grid = new SoundboardGridComponent(processor));
    grid->toFront(true);
    if (resizableCornerComponent)
    {
        resizableCornerComponent->toFront(false);
    }
    resized();
}

void SoundboardAudioProcessorEditor::buttonClicked(Button *buttonThatWasClicked)
{
    if (loadDirectoryButton == buttonThatWasClicked)
    {
        FileChooser chooser("Open...");
        if (chooser.browseForDirectory())
        {
            auto directory = chooser.getResult();
            if (directory.isDirectory())
            {
                showTable();
                processor.openDirectory(directory);
                table->updateContent();
            }
        }
    }
    else if (settingsButton == buttonThatWasClicked)
    {
        DialogWindow::LaunchOptions launchOptions;
        launchOptions.componentToCentreAround = this;
        launchOptions.content.setOwned(new SoundboardSettingsComponent(processor));
        launchOptions.content->setSize(640, 480);
        launchOptions.dialogBackgroundColour = Colours::black;
        launchOptions.dialogTitle = TRANS("Soundboard Settings");
        launchOptions.escapeKeyTriggersCloseButton = true;
        launchOptions.resizable = true;
        launchOptions.useNativeTitleBar = true;
        launchOptions.runModal();
    }
    else if (gridButton == buttonThatWasClicked)
    {
        showGrid();
    }
    else if (listButton == buttonThatWasClicked)
    {
        showTable();
    }
    else if (duckButton == buttonThatWasClicked) {
        bool ducking = processor.getOscManager()->getOscParameterValue("/ultraschall/soundboard/duck/enabled");
        processor.getOscManager()->setOscParameterValue("/ultraschall/soundboard/duck/enabled", !ducking);
        if (!ducking) {
            duckButton->setButtonText(FontAwesome_Comment);
        } else {
            duckButton->setButtonText(FontAwesome_CommentO);
        }
    }
    else if (lockButton == buttonThatWasClicked) {
        processor.toggleLocked();
        if (processor.getLocked()) {
            lockButton->setButtonText(FontAwesome_Lock);
        } else {
            lockButton->setButtonText(FontAwesome_Unlock);
        }
    }
}

void SoundboardAudioProcessorEditor::timerCallback(int timerID)
{
    if (timerID == TimerIdRefresh)
    {
        refresh();
    }
}

void SoundboardAudioProcessorEditor::refresh()
{
    if (!processor.isLocked())
    {
        if (table != nullptr) {
            table->updateContent();
        }
    }
}

void SoundboardAudioProcessorEditor::handleOscParameterMessage(OscParameter *parameter) {
    if (parameter->addressMatch("/ultraschall/soundboard/gain$")) {
        gainSlider->setValue(gainSlider->proportionOfLengthToValue(parameter->getValue()), dontSendNotification);
        AttributedString text;
        String value(gainSlider->getValue());
        if (value.length() == 1) {
            text.append("  ");
        } else if (value.length() == 2) {
            text.append(" ");
        }
        text.append(value, ThemeForeground1);
        text.append(" %", ThemeForeground1);
        gainBubble->toFront(false);
        gainBubble->showAt(gainSlider, text, 500);
    } else if(parameter->addressMatch("/ultraschall/soundboard/duck/gain$")) {
        gainSlider->setValue(gainSlider->proportionOfLengthToValue(parameter->getValue()), dontSendNotification);
    } else if(parameter->addressMatch("/ultraschall/soundboard/duck/enabled$")) {
        bool ducking = parameter->getValue();
        gainSlider->setEnabled(!ducking);
    }
}

void SoundboardAudioProcessorEditor::sliderValueChanged(Slider *slider)
{
    if (slider == gainSlider) {
        processor.getOscManager()->setOscParameterValue("/ultraschall/soundboard/gain", static_cast<float>(slider->valueToProportionOfLength(slider->getValue())));
    }
}
