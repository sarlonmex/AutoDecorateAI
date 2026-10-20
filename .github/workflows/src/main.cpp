#include <Geode/Geode.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <cocos2d.h>
#include <string>
#include <vector>
#include <map>

using namespace geode::prelude;

// ==========================================
// 1. MEHRSPRACHIGKEIT / MULTI-LANGUAGE
// ==========================================
enum class Language { German, English, Spanish, French, Chinese };

struct LocalizedText {
    std::string title;
    std::string styleLabel;
    std::string densityLabel;
    std::string promptPlaceholder;
    std::string generateBtn;
    std::string tutorialBtn;
    std::string skipTutorialBtn;
    std::string tutorialText;
};

std::map<Language, LocalizedText> g_translations = {
    { Language::German, {
        "Auto Decorate KI", "Stil wählen:", "Objekt-Anzahl / Dichte:", 
        "Beschreibe deinen Style (z.B. Cyberpunk, Neon, Deep Sea)...", "Level Dekorieren 🚀", 
        "Tutorial starten", "Tutorial überspringen",
        "Willkommen! 1. Wähle deinen Stil. 2. Gib deine Objektanzahl & Prompt ein. 3. Klicke auf 'Dekorieren'!"
    }},
    { Language::English, {
        "Auto Decorate AI", "Select Style:", "Object Count / Density:", 
        "Describe your style (e.g., Cyberpunk, Neon Ocean, Dark Void)...", "Decorate Level 🚀", 
        "Start Tutorial", "Skip Tutorial",
        "Welcome! 1. Pick a theme. 2. Set object density & prompt. 3. Click 'Decorate' to automatically generate decor!"
    }},
    { Language::Spanish, {
        "Auto Decorate IA", "Seleccionar estilo:", "Cantidad de objetos:", 
        "Describe tu estilo (ej. Neón, Océano, Cyberpunk)...", "Decorar nivel 🚀", 
        "Iniciar tutorial", "Omitir tutorial",
        "¡Bienvenido! 1. Elige un tema. 2. Ajusta la cantidad de objetos. 3. ¡Haz clic en 'Decorar'!"
    }},
    { Language::French, {
        "Auto Decorate IA", "Choisir le style:", "Nombre d'objets:", 
        "Décrivez votre style (ex: Cyberpunk, Océan, Sombre)...", "Décorer le niveau 🚀", 
        "Démarrer le tutoriel", "Passer le tutoriel",
        "Bienvenue! 1. Choisissez un style. 2. Définissez la densité. 3. Cliquez sur 'Décorer'!"
    }},
    { Language::Chinese, {
        "AI 自动装饰", "选择风格:", "物件数量/密度:", 
        "描述你的风格 (例如: 赛博朋克, 深海, 暗黑)...", "开始自动装饰 🚀", 
        "开始教程", "跳过教程",
        "欢迎！1. 选择样式 2. 输入提示词与密度 3. 点击'开始自动装饰'！"
    }}
};

// Globaler Mod-State
static Language g_currentLang = Language::German;
static bool g_tutorialCompleted = false;

// ==========================================
// 2. AUTO DECORATE ENGINE & KI LOGIK
// ==========================================
class AutoDecorateEngine {
public:
    enum class Theme { Light, Dark, Ocean, Cyberpunk, Forest, CustomAI };

    static void decorateLevel(LevelEditorLayer* editor, Theme theme, int objectTargetCount, const std::string& aiPrompt) {
        if (!editor || !editor->m_objects) return;

        std::vector<GameObject*> gameplayObjects;
        for (auto* obj : CCArrayExt<GameObject*>(editor->m_objects)) {
            if (obj) {
                gameplayObjects.push_back(obj);
            }
        }

        int addedObjects = 0;
        
        cocos2d::ccColor3B primaryColor = {255, 255, 255};
        cocos2d::ccColor3B glowColor = {0, 255, 255};

        if (theme == Theme::Ocean || aiPrompt.find("ocean") != std::string::npos || aiPrompt.find("wasser") != std::string::npos) {
            primaryColor = {0, 100, 255};
            glowColor = {0, 230, 255};
        } else if (theme == Theme::Dark || aiPrompt.find("dunkel") != std::string::npos || aiPrompt.find("dark") != std::string::npos) {
            primaryColor = {30, 30, 40};
            glowColor = {150, 0, 255};
        } else if (theme == Theme::Light) {
            primaryColor = {240, 240, 255};
            glowColor = {255, 220, 100};
        } else if (theme == Theme::Cyberpunk) {
            primaryColor = {255, 0, 128};
            glowColor = {0, 255, 200};
        }

        for (auto* baseObj : gameplayObjects) {
            if (addedObjects >= objectTargetCount) break;

            cocos2d::CCPoint pos = baseObj->getPosition();

            auto* glowObj = editor->createObject(1006, pos + cocos2d::CCPoint(0, 0), false);
            if (glowObj) {
                glowObj->setObjectColor(glowColor);
                addedObjects++;
            }

            if (addedObjects < objectTargetCount) {
                auto* detailObj = editor->createObject(1820, pos + cocos2d::CCPoint(0, 30), false);
                if (detailObj) {
                    detailObj->setObjectColor(primaryColor);
                    addedObjects++;
                }
            }
        }

        FLAlertLayer::create("Erfolg!", "Das Level wurde erfolgreich mit KI-Stil und Glow dekoriert!", "OK")->show();
    }
};

// ==========================================
// 3. UI POPUP (5 SPRACHEN + KI INTERFACE)
// ==========================================
class AutoDecoratePopup : public Popup<std::string const&> {
protected:
    CCMenuItemToggler* m_langBtns[5];
    TextInput* m_promptInput;
    Slider* m_densitySlider;
    CCLabelBMFont* m_densityLabel;
    int m_objectCount = 500;
    AutoDecorateEngine::Theme m_selectedTheme = AutoDecorateEngine::Theme::Ocean;

    bool setup(std::string const& value) override {
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto txt = g_translations[g_currentLang];

        this->setTitle(txt.title);

        auto langMenu = CCMenu::create();
        langMenu->setPosition({winSize.width / 2, winSize.height / 2 + 110});

        const char* langNames[5] = {"DE", "EN", "ES", "FR", "ZH"};
        for (int i = 0; i < 5; i++) {
            auto btnLbl = CCLabelBMFont::create(langNames[i], "bigFont.fnt");
            btnLbl->setScale(0.4f);
            auto btn = CCMenuItemExt::createSpriteExtra(btnLbl, [this, i](id) {
                g_currentLang = static_cast<Language>(i);
                this->onClose(nullptr);
                AutoDecoratePopup::create("")->show();
            });
            btn->setPosition({-100.0f + (i * 50.0f), 0.0f});
            langMenu->addChild(btn);
        }
        m_mainLayer->addChild(langMenu);

        if (!g_tutorialCompleted) {
            auto tutBg = CCScale9Sprite::create("square02_001.png", {0, 0, 80, 80});
            tutBg->setContentSize({340, 60});
            tutBg->setPosition({winSize.width / 2, winSize.height / 2 + 55});
            tutBg->setOpacity(180);
            m_mainLayer->addChild(tutBg);

            auto tutText = CCLabelBMFont::create(txt.tutorialText.c_str(), "chatFont.fnt");
            tutText->setScale(0.55f);
            tutText->setWidth(320);
            tutText->setPosition({winSize.width / 2, winSize.height / 2 + 55});
            m_mainLayer->addChild(tutText);

            auto skipBtnLbl = CCLabelBMFont::create(txt.skipTutorialBtn.c_str(), "goldFont.fnt");
            skipBtnLbl->setScale(0.4f);
            auto skipBtn = CCMenuItemExt::createSpriteExtra(skipBtnLbl, [](id) {
                g_tutorialCompleted = true;
            });
            skipBtn->setPosition({winSize.width / 2 + 120, winSize.height / 2 + 30});
            m_buttonMenu->addChild(skipBtn);
        }

        m_promptInput = TextInput::create(260.0f, txt.promptPlaceholder.c_str(), "chatFont.fnt");
        m_promptInput->setPosition({winSize.width / 2, winSize.height / 2 - 10});
        m_mainLayer->addChild(m_promptInput);

        m_densityLabel = CCLabelBMFont::create((txt.densityLabel + " 500").c_str(), "bigFont.fnt");
        m_densityLabel->setScale(0.35f);
        m_densityLabel->setPosition({winSize.width / 2, winSize.height / 2 - 50});
        m_mainLayer->addChild(m_densityLabel);

        auto genBtnSprite = ButtonSprite::create(txt.generateBtn.c_str(), "goldFont.fnt", "GJ_button_01.png", 0.8f);
        auto genBtn = CCMenuItemExt::createSpriteExtra(genBtnSprite, [this](id) {
            auto editor = LevelEditorLayer::get();
            if (editor) {
                std::string prompt = m_promptInput->getString();
                AutoDecorateEngine::decorateLevel(editor, m_selectedTheme, m_objectCount, prompt);
                this->onClose(nullptr);
            }
        });
        genBtn->setPosition({winSize.width / 2, winSize.height / 2 - 100});
        m_buttonMenu->addChild(genBtn);

        return true;
    }

public:
    static AutoDecoratePopup* create(std::string const& text) {
        auto ret = new AutoDecoratePopup();
        if (ret && ret->initAnchored(380, 280, text)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// ==========================================
// 4. EDITOR UI HOOK
// ==========================================
class $modify(MyEditorUI, EditorUI) {
    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) return false;

        auto btnSprite = CircleButtonSprite::createWithSpriteFrameName("GJ_paintBtn_001.png", 0.9f, CircleBaseColor::Green);
        auto btn = CCMenuItemExt::createSpriteExtra(btnSprite, [](id) {
            AutoDecoratePopup::create("")->show();
        });

        if (auto menu = this->getChildByID("settings-menu")) {
            menu->addChild(btn);
            menu->updateLayout();
        } else {
            this->addChild(btn);
        }

        return true;
    }
};
