#include "../../main.h"
#include "../gui.h"
#include "../../game/game.h"
#include "../../log.h"
#include "../settings.h"
#include "../java/jniutil.h"

extern CGame* pGame;
extern UI* pUI;
extern CSettings* pSettings;
extern CJavaWrapper* pJavaWrapper;

/* Keyboard */

Keyboard::Keyboard()
{
	m_input = new KeyboardInput();
	this->addChild(m_input);


	m_layoutLowerENG = new KeyboardLayoutLowerENG();
	this->addChild(m_layoutLowerENG);
	m_layoutLowerENG->setPadding(0.0f);
	m_layoutLowerENG->setSpacing(0.0f);
	m_layoutLowerENG->setVisible(false);

	m_layoutUpperENG = new KeyboardLayoutUpperENG();
	this->addChild(m_layoutUpperENG);
	m_layoutUpperENG->setPadding(0.0f);
	m_layoutUpperENG->setSpacing(0.0f);
	m_layoutUpperENG->setVisible(false);

	m_layoutLowerRU = new KeyboardLayoutLowerRU();
	this->addChild(m_layoutLowerRU);
	m_layoutLowerRU->setPadding(0.0f);
	m_layoutLowerRU->setSpacing(0.0f);
	m_layoutLowerRU->setVisible(false);

	m_layoutUpperRU = new KeyboardLayoutUpperRU();
	this->addChild(m_layoutUpperRU);
	m_layoutUpperRU->setPadding(0.0f);
	m_layoutUpperRU->setSpacing(0.0f);
	m_layoutUpperRU->setVisible(false);


	m_layoutNUM = new KeyboardLayoutNUM();
	this->addChild(m_layoutNUM);
	m_layoutNUM->setPadding(0.0f);
	m_layoutNUM->setSpacing(0.0f);
	m_layoutNUM->setVisible(false);

	m_activeLayout = m_layoutLowerENG;
	m_activeLayoutType = KeyboardLayout::Type::ENG_LOWER;
	m_activeLayout->setVisible(true);

	m_history = new KeyboardHistory();
}

void Keyboard::performLayout()
{
	m_input->setFixedSize(ImVec2(width(), UISettings::keyboardRowHeight()));
	m_input->setPosition(ImVec2(0.0f, 0.0f));

	/* ENG */
	m_layoutLowerENG->setFixedSize(ImVec2(width(), height() - m_input->height()+50.0f));
	m_layoutLowerENG->setPosition(ImVec2(0.0f, UISettings::keyboardRowHeight()));
	m_layoutUpperENG->setFixedSize(ImVec2(width(), height() - m_input->height()+50.0f));
	m_layoutUpperENG->setPosition(ImVec2(0.0f, UISettings::keyboardRowHeight()));

	/* RU */
	m_layoutLowerRU->setFixedSize(ImVec2(width(), height() - m_input->height()+50.0f));
	m_layoutLowerRU->setPosition(ImVec2(0.0f, UISettings::keyboardRowHeight()));
	m_layoutUpperRU->setFixedSize(ImVec2(width(), height() - m_input->height()+50.0f));
	m_layoutUpperRU->setPosition(ImVec2(0.0f, UISettings::keyboardRowHeight()));

	/* NUM */
	m_layoutNUM->setFixedSize(ImVec2(width(), height() - m_input->height()+50.0f));
	m_layoutNUM->setPosition(ImVec2(0.0f, UISettings::keyboardRowHeight()));

	Widget::performLayout();
}

void Keyboard::draw(ImGuiRenderer* renderer)
{
	renderer->drawRect(absolutePosition(), absolutePosition() + size(), UISettings::keyboardBackgroundColor(), true);
	Widget::draw(renderer);
}

void Keyboard::setActiveLayout(KeyboardLayout::Type type)
{
	m_activeLayout->setVisible(false);

	switch (type)
	{
		case KeyboardLayout::Type::ENG_LOWER:
			m_layoutLowerENG->setVisible(true);
			m_activeLayout = m_layoutLowerENG;
			break;

		case KeyboardLayout::Type::ENG_UPPER:
			m_layoutUpperENG->setVisible(true);
			m_activeLayout = m_layoutUpperENG;
			break;

		case KeyboardLayout::Type::RU_LOWER:
			m_layoutLowerRU->setVisible(true);
			m_activeLayout = m_layoutLowerRU;
			break;

		case KeyboardLayout::Type::RU_UPPER:
			m_layoutUpperRU->setVisible(true);
			m_activeLayout = m_layoutUpperRU;
			break;

		case KeyboardLayout::Type::NUM:
			m_layoutNUM->setVisible(true);
			m_activeLayout = m_layoutNUM;
			break;
	}

	m_activeLayoutType = type;
}

void Keyboard::changeCase(KeyboardLayout::KeyCase key_case)
{
	switch (key_case)
	{
		case KeyboardLayout::KeyCase::LOWER:
			if (m_activeLayoutType == KeyboardLayout::Type::ENG_UPPER) setActiveLayout(KeyboardLayout::Type::ENG_LOWER);

			if (m_activeLayoutType == KeyboardLayout::Type::RU_UPPER) setActiveLayout(KeyboardLayout::Type::RU_LOWER);

			break;

		case KeyboardLayout::KeyCase::UPPER:
			if (m_activeLayoutType == KeyboardLayout::Type::ENG_LOWER) setActiveLayout(KeyboardLayout::Type::ENG_UPPER);

			if (m_activeLayoutType == KeyboardLayout::Type::RU_LOWER) setActiveLayout(KeyboardLayout::Type::RU_UPPER);

			break;
	}


}

void Keyboard::show(Widget* caller)
{
	if(pSettings->Get().iAndroidKeyboard)
	{
		pJavaWrapper->ShowKeyboard();
	}
	else {
		m_history->reset();
		m_input->clear();
		this->setVisible(true);
		pGame->EnableGameInput(false);
	}
	m_caller = caller;
}

void Keyboard::hide()
{
	if(pSettings->Get().iAndroidKeyboard)
	{
		pJavaWrapper->HideKeyboard();
	}
	else {
		this->setVisible(false);
		pGame->EnableGameInput(true);
	}
	m_caller = nullptr;
}

void Keyboard::send()
{
	//if (m_caller) m_caller->keyboardEvent(this->input()->inputString());
	const std::string input = this->input()->inputString();

	m_history->add(input);
	if (m_caller) m_caller->keyboardEvent(input);
	this->hide();
}

void Keyboard::sendForGB(JNIEnv *pEnv, jobject thiz, jbyteArray str)
{
	std::string::size_type v8; // r0
	v8 = pEnv->functions->GetArrayLength(pEnv, (jarray)str);
	char* buffer = (char*)malloc(v8 + 1);
	jbyte* elements = pEnv->GetByteArrayElements(str, NULL);
	memcpy(buffer, elements, v8);
	buffer[v8] = 0;
	std::string input = std::string(buffer);
	if (m_caller) m_caller->keyboardEvent(input);
	hide();
	pEnv->ReleaseByteArrayElements(str, elements, JNI_ABORT);
}

void Keyboard::activateEvent(bool active)
{
	if (active == false) this->hide();
}

/* KeyboardInput */
KeyboardInput::KeyboardInput()
{
	m_buttonTub = new Button("/", UISettings::fontSize());
	this->addChild(m_buttonTub);
	m_buttonPrev = new Button("<<", UISettings::fontSize());
	this->addChild(m_buttonPrev);
	m_buttonNext = new Button(">>", UISettings::fontSize());
	this->addChild(m_buttonNext);

	m_buttonPrev->setCallback([]()
							  {
								  std::string text = pUI->keyboard()->history()->prev();
								  pUI->keyboard()->input()->setInputString(text);
							  }
	);

	m_buttonNext->setCallback([]()
							  {
								  std::string text = pUI->keyboard()->history()->next();
								  pUI->keyboard()->input()->setInputString(text);
							  }
	);

	m_buttonTub->setCallback([]()
							 {
								 //std::string text = pUI->keyboard()->history()->prev();
								 pUI->keyboard()->input()->setInputString("/");
							 }
	);

	m_buttonPrev->setVisible(true);
	m_buttonNext->setVisible(true);
	m_buttonTub->setVisible(true);
}

void KeyboardInput::performLayout()
{
	Widget::performLayout();

	m_buttonTub->setPosition(
			ImVec2(width() - (m_buttonTub->width() + UISettings::padding() + m_buttonPrev->width()*2.3 + UISettings::padding()),
				   (height() - m_buttonTub->height()) / 2));
	m_buttonPrev->setPosition(
			ImVec2(width() - (m_buttonPrev->width() + UISettings::padding() + m_buttonNext->width() + UISettings::padding()),
				   (height() - m_buttonPrev->height()) / 2));
	m_buttonNext->setPosition(
			ImVec2(width() - (m_buttonNext->width() + UISettings::padding()),
				   (height() - m_buttonNext->height()) / 2));
}

void KeyboardInput::addCharToInput(char value)
{
	m_input += value;
	m_caption = Encoding::cp2utf(m_input);
}

void KeyboardInput::popCharFromInput()
{
	if (m_input.empty()) return;

	m_input.pop_back();
	m_caption = Encoding::cp2utf(m_input);
}

void KeyboardInput::draw(ImGuiRenderer* renderer)
{
	float divLinePosY = (absolutePosition().y + size().y) - UISettings::outlineSize();
	float font_sz = UISettings::fontSize();

	renderer->drawText(absolutePosition() + ImVec2(UISettings::padding(), (height() - font_sz) / 2),
					   ImColor(1.0f, 1.0f, 1.0f), m_caption, false, font_sz);

	renderer->drawLine(
			ImVec2(absolutePosition().x, divLinePosY),
			ImVec2(absolutePosition().x + size().x, divLinePosY),
			ImColor(0xF5, 0x91, 0x32), UISettings::outlineSize()
	);

	Widget::draw(renderer);
}

/* KeyboardLayout */

KeyboardLayout::KeyboardLayout()
		: Layout(Orientation::VERTICAL)
{
	for (int i = 0; i < 5; i++)
	{
		m_rows[i] = new Row();
		this->addChild(m_rows[i]);
	}
}

void KeyboardLayout::performLayout()
{
	Layout::performLayout();
}

/* KeyboardLayout::Row */

KeyboardLayout::Row::Row()
		: Layout(Orientation::HORIZONTAL)
{

}

KeyboardLayout::Row::Key* KeyboardLayout::Row::addKey(const std::string&  caption, char value)
{
	Key* key = new Key(caption, value);
	this->addChild(key);
	return key;
}

KeyboardLayout::Row::Key* KeyboardLayout::Row::addKey(ExtendedKey extended_key)
{
	Key* key = new Key(extended_key);
	this->addChild(key);
	return key;
}

/* KeyboardLayout::Row::Key */

KeyboardLayout::Row::Key::Key(const std::string& caption, char value)
		: Button(caption)
{
	m_extendedKey = ExtendedKey::NONE;
	m_value = value;
}

KeyboardLayout::Row::Key::Key(ExtendedKey extended_key)
		: Button("")
{
	switch (extended_key)
	{
		case ExtendedKey::NUM:
			this->setCaption(std::string("123"));
			break;
		case ExtendedKey::LANG_ENG:
			this->setCaption(std::string("ENG"));
			break;

		case ExtendedKey::LANG_RU:
			this->setCaption(std::string("RUS"));
	}

	m_extendedKey = extended_key;
}

void KeyboardLayout::Row::Key::draw(ImGuiRenderer* renderer)
{
	if (focused()) renderer->drawRect(absolutePosition(), absolutePosition() + size(), ImColor(0xF5, 0x91, 0x32), true);

	switch (m_extendedKey)
	{
		case ExtendedKey::SHIFT_ON:
			renderer->drawTriangle(
					ImVec2(absolutePosition().x + width() * 0.37f, absolutePosition().y + height() * 0.50f),
					ImVec2(absolutePosition().x + width() * 0.50f, absolutePosition().y + height() * 0.25f),
					ImVec2(absolutePosition().x + width() * 0.63f, absolutePosition().y + height() * 0.50f),
					ImColor(1.0f, 1.0f, 1.0f), true
			);
			renderer->drawRect(
					ImVec2(absolutePosition().x + width() * 0.44f, absolutePosition().y + height() * 0.50f - 1),
					ImVec2(absolutePosition().x + width() * 0.56f, absolutePosition().y + height() * 0.68f),
					ImColor(1.0f, 1.0f, 1.0f), true
			);
			break;

		case ExtendedKey::SHIFT_OFF:
			renderer->drawTriangle(
					ImVec2(absolutePosition().x + width() * 0.37f, absolutePosition().y + height() * 0.50f),
					ImVec2(absolutePosition().x + width() * 0.50f, absolutePosition().y + height() * 0.25f),
					ImVec2(absolutePosition().x + width() * 0.63f, absolutePosition().y + height() * 0.50f),
					ImColor(0xF5, 0x91, 0x32), true
			);
			renderer->drawRect(
					ImVec2(absolutePosition().x + width() * 0.44f, absolutePosition().y + height() * 0.50f - 1),
					ImVec2(absolutePosition().x + width() * 0.56f, absolutePosition().y + height() * 0.68f),
					ImColor(0xF5, 0x91, 0x32), true
			);
			break;

		case ExtendedKey::BACKSPACE:
			static ImVec2 points[5];
			points[0] = ImVec2(absolutePosition().x + width() * 0.35f, absolutePosition().y + height() * 0.50f);
			points[1] = ImVec2(absolutePosition().x + width() * 0.45f, absolutePosition().y + height() * 0.25f);
			points[2] = ImVec2(absolutePosition().x + width() * 0.65f, absolutePosition().y + height() * 0.25f);
			points[3] = ImVec2(absolutePosition().x + width() * 0.65f, absolutePosition().y + height() * 0.65f);
			points[4] = ImVec2(absolutePosition().x + width() * 0.45f, absolutePosition().y + height() * 0.65f);
			renderer->drawConvexPolyFilled(points, 5, ImColor(1.0f, 1.0f, 1.0f));
			break;

		case ExtendedKey::SPACE:
			renderer->drawRect(
					ImVec2(absolutePosition().x + width() * 0.07f, absolutePosition().y + height() * 0.35f),
					ImVec2(absolutePosition().x + width() * 0.93f, absolutePosition().y + height() * 0.65f),
					ImColor(1.0f, 1.0f, 1.0f), true
			);
			break;

		case ExtendedKey::SEND:
			renderer->drawTriangle(
					ImVec2(absolutePosition().x + width() * 0.3f, absolutePosition().y + height() * 0.25f),
					ImVec2(absolutePosition().x + width() * 0.3f, absolutePosition().y + height() * 0.75f),
					ImVec2(absolutePosition().x + width() * 0.7f, absolutePosition().y + height() * 0.50f),
					ImColor(1.0f, 1.0f, 1.0f), true
			);
			break;
	}

	Widget::draw(renderer);
}

void KeyboardLayout::Row::Key::touchPopEvent()
{
	Keyboard* keyboard = dynamic_cast<Keyboard*>(this->parent()->parent()->parent());

	switch (m_extendedKey)
	{
		case ExtendedKey::BACKSPACE:
			keyboard->input()->popCharFromInput();
			break;

		case ExtendedKey::SPACE:
			keyboard->input()->addCharToInput(' ');
			break;

		case ExtendedKey::SEND:
			keyboard->send();
			break;

		case ExtendedKey::SHIFT_ON:
			keyboard->changeCase(KeyCase::UPPER);
			break;

		case ExtendedKey::SHIFT_OFF:
			keyboard->changeCase(KeyCase::LOWER);
			break;

		case ExtendedKey::LANG_RU:
			keyboard->setActiveLayout(KeyboardLayout::Type::RU_LOWER);
			break;

		case ExtendedKey::LANG_ENG:
			keyboard->setActiveLayout(KeyboardLayout::Type::ENG_LOWER);
			break;

		case ExtendedKey::NUM:
			keyboard->setActiveLayout(KeyboardLayout::Type::NUM);
			break;

		case ExtendedKey::NONE:
			keyboard->input()->addCharToInput(m_value);
			break;
	}
}

/* KeyboardLayoutLowerENG */

KeyboardLayoutLowerENG::KeyboardLayoutLowerENG()
		: KeyboardLayout()
{

}

void KeyboardLayoutLowerENG::performLayout()
{
	m_rows[0]->setPadding(0.0f);
	m_rows[0]->setSpacing(0.0f);
	m_rows[0]->addKey(std::string ("q"), 0);
	m_rows[0]->addKey(std::string ("w"), 0);
	m_rows[0]->addKey(std::string ("e"), 0);
	m_rows[0]->addKey(std::string ("r"), 0);
	m_rows[0]->addKey(std::string ("t"), 0);
	m_rows[0]->addKey(std::string ("y"), 0);
	m_rows[0]->addKey(std::string ("u"), 0);
	m_rows[0]->addKey(std::string ("i"), 0);
	m_rows[0]->addKey(std::string ("o"), 0);
	m_rows[0]->addKey(std::string ("p"), 0);

	ImVec2 keySize = ImVec2(width() / 10, UISettings::keyboardRowHeight());
	float padding = keySize.x / 2;
	m_rows[1]->setPadding(0.0f);
	m_rows[1]->setSpacing(0.0f);
	m_rows[1]->setPaddingLeft(padding);
	m_rows[1]->setPaddingRight(padding);
	m_rows[1]->addKey(std::string ("a"), 0);
	m_rows[1]->addKey(std::string ("s"), 0);
	m_rows[1]->addKey(std::string ("d"), 0);
	m_rows[1]->addKey(std::string ("f"), 0);
	m_rows[1]->addKey(std::string ("g"), 0);
	m_rows[1]->addKey(std::string ("h"), 0);
	m_rows[1]->addKey(std::string ("j"), 0);
	m_rows[1]->addKey(std::string ("k"), 0);
	m_rows[1]->addKey(std::string ("l"), 0);

	m_rows[2]->setPadding(0.0f);
	m_rows[2]->setSpacing(0.0f);
	m_rows[2]->addKey(ExtendedKey::SHIFT_ON)->setFixedSize(ImVec2(keySize.x * 1.5f, 0.0f));
	m_rows[2]->addKey(std::string ("z"), 0);
	m_rows[2]->addKey(std::string ("x"), 0);
	m_rows[2]->addKey(std::string ("c"), 0);
	m_rows[2]->addKey(std::string ("v"), 0);
	m_rows[2]->addKey(std::string ("b"), 0);
	m_rows[2]->addKey(std::string ("n"), 0);
	m_rows[2]->addKey(std::string ("m"), 0);
	m_rows[2]->addKey(ExtendedKey::BACKSPACE)->setFixedSize(ImVec2(keySize.x * 1.5f, 0.0f));

	m_rows[3]->setPadding(0.0f);
	m_rows[3]->setSpacing(0.0f);
	/*m_rows[4]->addKey(std::string ("/", '/')->setFixedSize(ImVec2(keySize.x * 1.5f, 0.0f));
	m_rows[4]->addKey(ExtendedKey::NUM);
	m_rows[4]->addKey(ExtendedKey::LANG_RU);*/
	m_rows[3]->addKey(ExtendedKey::NUM);
	m_rows[3]->addKey(ExtendedKey::LANG_ENG);
	m_rows[3]->addKey(ExtendedKey::LANG_RU);
	m_rows[3]->addKey(ExtendedKey::SPACE)->setFixedSize(ImVec2(keySize.x * 3, 0.0f));
	m_rows[3]->addKey(std::string ("/"), 0);
	m_rows[3]->addKey(std::string ("?"), 0);
	m_rows[3]->addKey(std::string ("."), 0);
	m_rows[3]->addKey(ExtendedKey::SEND)->setFixedSize(ImVec2(keySize.x * 1.5f, 0.0f));

	KeyboardLayout::performLayout();
}

/* KeyboardLayoutUpperENG */

KeyboardLayoutUpperENG::KeyboardLayoutUpperENG()
		: KeyboardLayout()
{

}

void KeyboardLayoutUpperENG::performLayout()
{
	m_rows[0]->setPadding(0.0f);
	m_rows[0]->setSpacing(0.0f);
	m_rows[0]->addKey(std::string ("Q"), 0);
	m_rows[0]->addKey(std::string ("W"), 0);
	m_rows[0]->addKey(std::string ("E"), 0);
	m_rows[0]->addKey(std::string ("R"), 0);
	m_rows[0]->addKey(std::string ("T"), 0);
	m_rows[0]->addKey(std::string ("Y"), 0);
	m_rows[0]->addKey(std::string ("U"), 0);
	m_rows[0]->addKey(std::string ("I"), 0);
	m_rows[0]->addKey(std::string ("O"), 0);
	m_rows[0]->addKey(std::string ("P"), 0);

	ImVec2 keySize = ImVec2(width() / 10, UISettings::keyboardRowHeight());
	float padding = keySize.x / 2;
	m_rows[1]->setPadding(0.0f);
	m_rows[1]->setSpacing(0.0f);
	m_rows[1]->setPaddingLeft(padding);
	m_rows[1]->setPaddingRight(padding);
	m_rows[1]->addKey(std::string ("A"), 0);
	m_rows[1]->addKey(std::string ("S"), 0);
	m_rows[1]->addKey(std::string ("D"), 0);
	m_rows[1]->addKey(std::string ("F"), 0);
	m_rows[1]->addKey(std::string ("G"), 0);
	m_rows[1]->addKey(std::string ("H"), 0);
	m_rows[1]->addKey(std::string ("J"), 0);
	m_rows[1]->addKey(std::string ("K"), 0);
	m_rows[1]->addKey(std::string ("L"), 0);

	m_rows[2]->setPadding(0.0f);
	m_rows[2]->setSpacing(0.0f);
	m_rows[2]->addKey(ExtendedKey::SHIFT_OFF)->setFixedSize(ImVec2(keySize.x * 1.5f, 0.0f));
	m_rows[2]->addKey(std::string ("Z"), 0);
	m_rows[2]->addKey(std::string ("X"), 0);
	m_rows[2]->addKey(std::string ("C"), 0);
	m_rows[2]->addKey(std::string ("V"), 0);
	m_rows[2]->addKey(std::string ("B"), 0);
	m_rows[2]->addKey(std::string ("N"), 0);
	m_rows[2]->addKey(std::string ("M"), 0);
	m_rows[2]->addKey(ExtendedKey::BACKSPACE)->setFixedSize(ImVec2(keySize.x * 1.5f, 0.0f));

	m_rows[3]->setPadding(0.0f);
	m_rows[3]->setSpacing(0.0f);
	m_rows[3]->addKey(ExtendedKey::NUM);//m_rows[4]->addKey(std::string ("/", '/')->setFixedSize(ImVec2(keySize.x * 1.5f, 0.0f));
	m_rows[3]->addKey(ExtendedKey::LANG_ENG);
	m_rows[3]->addKey(ExtendedKey::LANG_RU);
	m_rows[3]->addKey(ExtendedKey::SPACE)->setFixedSize(ImVec2(keySize.x * 3, 0.0f));
	m_rows[3]->addKey(std::string ("/"), 0);//m_rows[4]->addKey(std::string (",", ',');
	m_rows[3]->addKey(std::string ("?"), 0);
	m_rows[3]->addKey(std::string ("."), 0);
	m_rows[3]->addKey(ExtendedKey::SEND)->setFixedSize(ImVec2(keySize.x * 1.5f, 0.0f));

	KeyboardLayout::performLayout();
}

/* KeyboardLayoutLowerRU */

KeyboardLayoutLowerRU::KeyboardLayoutLowerRU()
		: KeyboardLayout()
{

}

void KeyboardLayoutLowerRU::performLayout()
{
	m_rows[0]->setPadding(0.0f);
	m_rows[0]->setSpacing(0.0f);
	m_rows[0]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[0]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[0]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[0]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[0]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[0]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[0]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[0]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[0]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[0]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[0]->addKey(Encoding::cp2utf("�"), 0);

	m_rows[1]->setPadding(0.0f);
	m_rows[1]->setSpacing(0.0f);
	m_rows[1]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[1]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[1]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[1]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[1]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[1]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[1]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[1]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[1]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[1]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[1]->addKey(Encoding::cp2utf("�"), 0);

	ImVec2 keySize = ImVec2(width() / 11, UISettings::keyboardRowHeight());
	m_rows[2]->setPadding(0.0f);
	m_rows[2]->setSpacing(0.0f);
	m_rows[2]->addKey(ExtendedKey::SHIFT_ON)->setFixedSize(ImVec2(keySize.x * 1.5f, 0.0f));
	m_rows[2]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[2]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[2]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[2]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[2]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[2]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[2]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[2]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[2]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[2]->addKey(ExtendedKey::BACKSPACE)->setFixedSize(ImVec2(keySize.x * 1.5f, 0.0f));

	m_rows[3]->setPadding(0.0f);
	m_rows[3]->setSpacing(0.0f);
	//m_rows[4]->addKey(std::string ("/", '/')->setFixedSize(ImVec2(keySize.x * 1.5f, 0.0f));
	//m_rows[4]->addKey(ExtendedKey::NUM);
	//m_rows[4]->addKey(ExtendedKey::LANG_ENG);
	m_rows[3]->addKey(ExtendedKey::NUM);
	m_rows[3]->addKey(ExtendedKey::LANG_ENG);
	m_rows[3]->addKey(ExtendedKey::LANG_RU);
	m_rows[3]->addKey(ExtendedKey::SPACE)->setFixedSize(ImVec2(keySize.x * 3, 0.0f));
	m_rows[3]->addKey(std::string ("/"), 0); //m_rows[4]->addKey(std::string (",", ',');
	m_rows[3]->addKey(std::string ("?"), 0);
	m_rows[3]->addKey(std::string ("."), 0);
	m_rows[3]->addKey(ExtendedKey::SEND)->setFixedSize(ImVec2(keySize.x * 1.5f, 0.0f));

	KeyboardLayout::performLayout();
}

/* KeyboardLayoutUpperRU*/

KeyboardLayoutUpperRU::KeyboardLayoutUpperRU()
		: KeyboardLayout()
{

}

void KeyboardLayoutUpperRU::performLayout()
{
	m_rows[0]->setPadding(0.0f);
	m_rows[0]->setSpacing(0.0f);
	m_rows[0]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[0]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[0]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[0]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[0]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[0]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[0]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[0]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[0]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[0]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[0]->addKey(Encoding::cp2utf("�"), 0);

	m_rows[1]->setPadding(0.0f);
	m_rows[1]->setSpacing(0.0f);
	m_rows[1]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[1]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[1]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[1]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[1]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[1]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[1]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[1]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[1]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[1]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[1]->addKey(Encoding::cp2utf("�"), 0);

	ImVec2 keySize = ImVec2(width() / 11, UISettings::keyboardRowHeight());
	m_rows[2]->setPadding(0.0f);
	m_rows[2]->setSpacing(0.0f);
	m_rows[2]->addKey(ExtendedKey::SHIFT_ON)->setFixedSize(ImVec2(keySize.x * 1.5f, 0.0f));
	m_rows[2]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[2]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[2]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[2]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[2]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[2]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[2]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[2]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[2]->addKey(Encoding::cp2utf("�"), 0);
	m_rows[2]->addKey(ExtendedKey::BACKSPACE)->setFixedSize(ImVec2(keySize.x * 1.5f, 0.0f));

	m_rows[3]->setPadding(0.0f);
	m_rows[3]->setSpacing(0.0f);
	//m_rows[4]->addKey(std::string ("/", '/')->setFixedSize(ImVec2(keySize.x * 1.5f, 0.0f));
	//m_rows[4]->addKey(ExtendedKey::NUM);
	//m_rows[4]->addKey(ExtendedKey::LANG_ENG);
	m_rows[3]->addKey(ExtendedKey::NUM);
	m_rows[3]->addKey(ExtendedKey::LANG_ENG);
	m_rows[3]->addKey(ExtendedKey::LANG_RU);
	m_rows[3]->addKey(ExtendedKey::SPACE)->setFixedSize(ImVec2(keySize.x * 3, 0.0f));
	m_rows[3]->addKey(std::string ("/"), 0); //m_rows[4]->addKey(std::string (",", ',');
	m_rows[3]->addKey(std::string ("?"), 0);
	m_rows[3]->addKey(std::string ("."), 0);
	m_rows[3]->addKey(ExtendedKey::SEND)->setFixedSize(ImVec2(keySize.x * 1.5f, 0.0f));

	KeyboardLayout::performLayout();
}

/* KeyboardLayoutNUM */

KeyboardLayoutNUM::KeyboardLayoutNUM()
		: KeyboardLayout()
{

}

void KeyboardLayoutNUM::performLayout()
{
	m_rows[0]->setPadding(0.0f);
	m_rows[0]->setSpacing(0.0f);
	m_rows[0]->addKey(std::string ("1"), 0);
	m_rows[0]->addKey(std::string ("2"), 0);
	m_rows[0]->addKey(std::string ("3"), 0);
	m_rows[0]->addKey(std::string ("4"), 0);
	m_rows[0]->addKey(std::string ("5"), 0);
	m_rows[0]->addKey(std::string ("6"), 0);
	m_rows[0]->addKey(std::string ("7"), 0);
	m_rows[0]->addKey(std::string ("8"), 0);
	m_rows[0]->addKey(std::string ("9"), 0);
	m_rows[0]->addKey(std::string ("0"), 0);

	m_rows[1]->setPadding(0.0f);
	m_rows[1]->setSpacing(0.0f);
	m_rows[1]->addKey(std::string ("@"), 0);
	m_rows[1]->addKey(std::string ("#"), 0);
	m_rows[1]->addKey(std::string ("$"), 0);
	m_rows[1]->addKey(std::string ("%"), 0);
	m_rows[1]->addKey(std::string ("\""), 0);
	m_rows[1]->addKey(std::string ("*"), 0);
	m_rows[1]->addKey(std::string ("("), 0);
	m_rows[1]->addKey(std::string (")"), 0);
	m_rows[1]->addKey(std::string ("-"), 0);
	m_rows[1]->addKey(std::string ("_"), 0);

	m_rows[2]->setPadding(0.0f);
	m_rows[2]->setSpacing(0.0f);
	m_rows[2]->addKey(std::string ("."), 0);
	m_rows[2]->addKey(std::string (":"), 0);
	m_rows[2]->addKey(std::string (";"), 0);
	m_rows[2]->addKey(std::string ("+"), 0);
	m_rows[2]->addKey(std::string ("="), 0);
	m_rows[2]->addKey(std::string ("<"), 0);
	m_rows[2]->addKey(std::string (">"), 0);
	m_rows[2]->addKey(std::string ("["), 0);
	m_rows[2]->addKey(std::string ("]"), 0);
	m_rows[2]->addKey(ExtendedKey::BACKSPACE);

	ImVec2 keySize = ImVec2(width() / 10, UISettings::keyboardRowHeight());

	m_rows[3]->setPadding(0.0f);
	m_rows[3]->setSpacing(0.0f);
	//m_rows[4]->addKey(std::string ("/", '/');
	//m_rows[4]->addKey(ExtendedKey::LANG_ENG);
	//m_rows[4]->addKey(std::string (",", ',');
	m_rows[3]->addKey(ExtendedKey::NUM);
	m_rows[3]->addKey(ExtendedKey::LANG_ENG);
	m_rows[3]->addKey(ExtendedKey::LANG_RU);
	m_rows[3]->addKey(ExtendedKey::SPACE)->setFixedSize(ImVec2(keySize.x * 4, 0.0f));
	m_rows[3]->addKey(std::string ("/"), 0);//m_rows[4]->addKey(std::string ("?", '?');
	m_rows[3]->addKey(std::string ("?"), 0);
	m_rows[3]->addKey(std::string ("!"), 0);
	m_rows[3]->addKey(ExtendedKey::SEND);

	KeyboardLayout::performLayout();
}

KeyboardHistory::KeyboardHistory()
{
	m_index = 0;
	m_count = 0;
	m_vMessages.resize(KEYBOARD_MAX_HISTORY_MESSAGES);
}

void KeyboardHistory::add(const std::string& input)
{
	for (int i = KEYBOARD_MAX_HISTORY_MESSAGES - 2; i > 0; i--)
	{
		std::string str = m_vMessages[i];
		m_vMessages[i] = m_vMessages[i - 1];
		m_vMessages[i - 1] = str;
	}

	m_vMessages[0] = input;
	if (m_count < KEYBOARD_MAX_HISTORY_MESSAGES) m_count++;
}

void KeyboardHistory::reset()
{
	m_index = 0;
}

const std::string KeyboardHistory::next()
{
	if (m_count == 0) return "";

	std::string str = m_vMessages[m_index];

	if (m_index < m_count - 1) m_index++;
	return str;
}

const std::string KeyboardHistory::prev()
{
	if (m_count == 0) return "";

	std::string str = m_vMessages[m_index];
	if (m_index > 0) m_index--;
	return str;
}