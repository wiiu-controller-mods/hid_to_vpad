/****************************************************************************
 * Copyright (C) 2016,2017 Maschell
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include <fs/DirList.h>
#include <utils/StringTools.h>

#include "ContentSettings.h"
#include "settings/CSettings.h"
#include "common/common.h"
#include "Application.h"

ContentSettings::ContentSettings():ContentTemplate()
    , touchTrigger(GuiTrigger::CHANNEL_1, GuiTrigger::VPAD_TOUCH)
    , wpadTouchTrigger(GuiTrigger::CHANNEL_2 | GuiTrigger::CHANNEL_3 | GuiTrigger::CHANNEL_4 | GuiTrigger::CHANNEL_5, GuiTrigger::BUTTON_A)
    , languageSelectBox("",NULL)
    , rumbleSwitch(CSettings::instance()->getValueAsBool(CSettings::RumbleActivated))
    , musicSwitch(CSettings::instance()->getValueAsBool(CSettings::MusicActivated))
    , networkControllerSwitch(CSettings::instance()->getValueAsBool(CSettings::NetworkControllerActivated))
    , buttonUpTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_UP | GuiTrigger::STICK_L_UP, true)
    , buttonDownTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_DOWN | GuiTrigger::STICK_L_DOWN, true)
    , buttonATrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_A, true)
    , DPADButtons(0, 0)
    , AButton(0, 0)
    , buttonClickSound(Resources::GetSound("settings_click_2.mp3"))
    {
    headLine.setText(gettext("Settings"));
    headLine.setFontSize(60);
    headLine.setAlignment(ALIGN_TOP_CENTER);
    headLine.setPosition(0,-110);
    headLine.setColor(glm::vec4(0.3f,0.3f,0.3f,1.0f));
    append(&headLine);

    settingsFrame.setAlignment(ALIGN_TOP_CENTER);
    settingsFrame.setPosition(0,-80);
    settingsFrame.setSize(getWidth(),getHeight());
    append(&settingsFrame);

    rumbleSwitch.setScale(2.0f);
    musicSwitch.setScale(2.0f);
    networkControllerSwitch.setScale(2.0f);

    settings[gettext("Language")] = &languageSelectBox;
    settings[gettext("Rumble")] = &rumbleSwitch;
    settings[gettext("Music")] = &musicSwitch;
    settings[gettext("Network Controller")] = &networkControllerSwitch;

    settingsOrder.push_back(gettext("Language"));
    settingsOrder.push_back(gettext("Rumble"));
    settingsOrder.push_back(gettext("Music"));
    settingsOrder.push_back(gettext("Network Controller"));

    rumbleSwitch.setTrigger(&touchTrigger);
    rumbleSwitch.setTrigger(&wpadTouchTrigger);
    rumbleSwitch.setSoundClick(buttonClickSound);
    rumbleSwitch.valueChanged.connect(this, &ContentSettings::OnRumbleValueChanged);

    musicSwitch.setTrigger(&touchTrigger);
    musicSwitch.setTrigger(&wpadTouchTrigger);
    musicSwitch.setSoundClick(buttonClickSound);
    musicSwitch.valueChanged.connect(this, &ContentSettings::OnMusicValueChanged);

    networkControllerSwitch.setTrigger(&touchTrigger);
    networkControllerSwitch.setTrigger(&wpadTouchTrigger);
    networkControllerSwitch.setSoundClick(buttonClickSound);
    networkControllerSwitch.valueChanged.connect(this, &ContentSettings::OnNetworkControllerValueChanged);

    DirList dirList(DEFAULT_LANG_PATH, LANGUAGE_FILE_EXT, DirList::Files);
    dirList.SortList();

    //Fill in the values for the select box
    languagesNames[gettext("<Default>")] = "none";
    int selectedID = 0;
    std::string setting = CSettings::instance()->getValueAsString(CSettings::AppLanguage);
    for(int i = 0; i < dirList.GetFilecount(); i++){
        const char *filename = dirList.GetFilename(i);
        std::string name(filename);

        name = name.substr(0,name.length()-strlen(LANGUAGE_FILE_EXT));
        std::string displayname = name;
        displayname[0] = toupper(displayname[0]);
        languagesNames[displayname] = name;
        if(name.compare(setting) == 0) selectedID = i+1;
    }

    languageSelectBox.Init(languagesNames,selectedID);
    languageSelectBox.valueChanged.connect(this, &ContentSettings::OnSelectBoxValueChanged);
    languageSelectBox.showhide.connect(this, &ContentSettings::OnLanguageBoxShowHide);

    f32 frameoffset = 0;
    f32 frameheight = 80.0f;
    for (std::vector<std::string>::iterator it = settingsOrder.begin() ; it != settingsOrder.end(); ++it){
        std::string name = *it;
        GuiElement * element =settings[*it];
        GuiFrame * frame = new GuiFrame();

        GuiFrame * left = new GuiFrame();
        GuiFrame * right = new GuiFrame();

        frame->append(left);
        frame->append(right);
        frame->setAlignment(ALIGN_TOP_CENTER);
        frame->setSize(getWidth()*0.80f,frameheight);

        GuiText * text = new GuiText(StringTools::strfmt("%s:",name.c_str()).c_str());
        text->setColor(glm::vec4(0.3f,0.3f,0.3f,1.0f));
        text->setFontSize(40);
        text->setAlignment(ALIGN_LEFT);
        left->append(text);
        left->setAlignment(ALIGN_LEFT);
        right->setAlignment(ALIGN_RIGHT);
        element->setAlignment(ALIGN_RIGHT);
        element->setPosition(0,10);

        right->append(element);

        frameoffset -= frameheight;
        frame->setPosition(0,frameoffset);
        settingsFrame.append(frame);
        settingsFrames[element] = frame;

        toDelete.push_back(frame);
        toDelete.push_back(left);
        toDelete.push_back(right);
        toDelete.push_back(text);
    }
    settingsFrame.bringToFront(settingsFrames.at(&languageSelectBox));

    DPADButtons.setTrigger(&buttonDownTrigger);
    DPADButtons.setTrigger(&buttonUpTrigger);
    DPADButtons.clicked.connect(this, &ContentSettings::OnDPADClick);
    append(&DPADButtons);

    AButton.setTrigger(&buttonATrigger);
    AButton.clicked.connect(this, &ContentSettings::OnAClick);
    append(&AButton);
}

ContentSettings::~ContentSettings(){
    remove(&headLine);
    remove(&settingsFrame);
    Resources::RemoveSound(buttonClickSound);

    if(bChanged){
        CSettings::instance()->Save();
    }

    for (std::vector<GuiElement*>::iterator it = toDelete.begin() ; it != toDelete.end(); ++it){
        GuiElement * element = *it;
        delete element;
    }
}

void ContentSettings::selectionChanged()
{
    if (isSelected)
    {
        settings[settingsOrder[0]]->setState(STATE_SELECTED);
    }
    else
    {
        for (unsigned int i = 0; i < settingsOrder.size(); i++)
        {
            settings[settingsOrder[i]]->clearState(STATE_SELECTED);
        }
    }
    
}

void ContentSettings::OnSelectBoxValueChanged(GuiSelectBox * selectBox, std::string value){
    if(selectBox == &languageSelectBox){
        if(CSettings::getValueAsString(CSettings::AppLanguage).compare(value) != 0){
            CSettings::setValueAsString(CSettings::AppLanguage,value);
            bChanged = true;
            Application::instance()->reloadUI();
        }
    }
}

void ContentSettings::OnMusicValueChanged(GuiToggle * toggle,bool value){
    CSettings::setValueAsBool(CSettings::MusicActivated,value);
    bChanged = true;
}


void ContentSettings::OnRumbleValueChanged(GuiToggle * toggle,bool value){
    CSettings::setValueAsBool(CSettings::RumbleActivated,value);
    bChanged = true;
}

void ContentSettings::OnNetworkControllerValueChanged(GuiToggle * toggle,bool value){
    CSettings::setValueAsBool(CSettings::NetworkControllerActivated,value);
    ControllerPatcher::setNetworkControllerActivated(value);
    if(!value){
        ControllerPatcher::stopNetworkServer();
    }else{
        ControllerPatcher::startNetworkServer();
    }
    bChanged = true;
}

std::string ContentSettings::getSelectedSetting()
{
    for (unsigned int i = 0; i < settingsOrder.size(); i++)
    {
        GuiElement* element = settings[settingsOrder[i]];
        if (element->isStateSet(STATE_SELECTED))
            return settingsOrder[i];
    }

    return "";
}

int ContentSettings::settingToIndex(std::string setting)
{
    for (unsigned int i = 0; i < settingsOrder.size(); i++)
    {
        if (settingsOrder[i] == setting)
            return i;
    }
    return -1;
}

void ContentSettings::OnLanguageBoxShowHide(GuiSelectBox* selectBox, bool showValues)
{
    isInSelectBox = showValues;
}

void ContentSettings::OnDPADClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    if (isInSelectBox)
        return;

    std::string setting = getSelectedSetting();
    int index = settingToIndex(setting);
    if (!setting.empty() && index >= 0)
    {
        if (trigger == &buttonDownTrigger && index < (int) settingsOrder.size() - 1)
        {
            settings[setting]->clearState(STATE_SELECTED);
            settings[settingsOrder[index + 1]]->setState(STATE_SELECTED);
        }
        if (trigger == &buttonUpTrigger && index > 0)
        {
            settings[setting]->clearState(STATE_SELECTED);
            settings[settingsOrder[index - 1]]->setState(STATE_SELECTED);
        }
    }
}

void ContentSettings::OnAClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    if (isInSelectBox)
        return;

    std::string setting = getSelectedSetting();
    if (!setting.empty())
    {
        DefaultGuiSwitch* gui_switch = dynamic_cast<DefaultGuiSwitch*>(settings[setting]);
        if (gui_switch != NULL)
        {
            gui_switch->clicked(gui_switch, controller, trigger);
        }

        DefaultGuiSelectBox* select_box = dynamic_cast<DefaultGuiSelectBox*>(settings[setting]);
        if (select_box != NULL)
        {
            select_box->OnTopValueClicked(NULL, NULL, NULL);
        }
    }
}