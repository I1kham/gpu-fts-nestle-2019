// **************************************************************************************
//
// K E Y B O A R D   L A Y O U T 
// https://keyshorts.com/blogs/blog/44712961-how-to-identify-laptop-keyboard-localization
//
// **************************************************************************************

const Keyboard = {
    elements: {
        main: null,
        langs: [],
        keysContainer: null,
        keys: [],
        textElement: null,
        root: ''
    },

    eventHandlers: {
        oninput: null,
        onclose: null
    },

    properties: {
        value: "",
        capsLock: 0,
        altGr: false,
        accent: false,
        carretPosition: 0
    },

    init(layout, root) {
		
        this._resetKeyboard();

        // Create main elements
        this.elements.main = document.createElement("div");
        this.elements.keysContainer = document.createElement("div");
        this.elements.root = this.elements.root || root;

        // Setup main elements
        this.elements.main.classList.add("keyboard", "keyboard--hidden");
        this.elements.main.dataset.rheaKeyboardElement = true;
        this.elements.keysContainer.classList.add("keyboard__keys");

        const textReference = //html
            `
                <div class="text-reference">
                    <div class="txt">&nbsp;</div>
                </div>
            `;

        this.elements.keysContainer.appendChild(this._stringToHTML(textReference));
        this.elements.keysContainer.appendChild(this._createKeys(layout));

        Object.keys(charset).forEach(lang => { this.elements.langs.push(lang); });

        this.elements.keys = this.elements.keysContainer.querySelectorAll(".keyboard__key");

        // Add to DOM
        this.elements.main.appendChild(this.elements.keysContainer);

        let options = '';

        this.elements.langs.forEach(lang => {
            const selected = lang === layout ? 'selected' : '';
            options += //html 
                `
                    <option value="${lang}" ${selected}>${lang}</option>
                `;
        });

        const selection = //html
            `
                <div class="layout-type-container">
                    <label for="langs">Keyboard layout</label>
                    <br>
                    <select data-root="${this.elements.root || ''}" name="langs" id="langs">
                        ${options}
                    </select>
                </div>
            `;

        this.elements.main.appendChild(this._stringToHTML(selection));
        this.elements.main.querySelector('#langs').onchange = (e) => {
            this.init(e.target.value, e.target.dataset.root);
        };

        document.body.appendChild(this.elements.main);
        document.addEventListener('click', (e) => {
            if (!this.elements.textElement) return;
            if (e.target.closest('[data-rhea-keyboard-element="true"]') ||
                this.elements.textElement.contains(e.target) || this.elements.textElement === e.target) {
                return;
            }

            this.close();
        });

        // Automatically use keyboard for elements with .use-keyboard-input
        document.querySelectorAll(".use-keyboard-input").forEach(element => {
			
            element.addEventListener("focus", () => {
                this.elements.textElement = element;

                this.open(element.value, currentValue => {
                    if (element.type === 'number') {
                        if (currentValue.match(/[^$,.\d]/)) {
                            this.properties.carretPosition--;
                        }

                        currentValue = currentValue.replace(/\D/g, '');

                        element.value = currentValue;
                        this.properties.value = currentValue;
                    } else {
                        element.value = currentValue;
                    }
                });
            });
        });

        // Automatically use keyboard for elements with .use-keyboard-input
        document.querySelectorAll(".use-keyboard-input").forEach(element => {
            element.addEventListener("click", (event) => {
                this.properties.carretPosition = event.target.selectionStart || event.target.value.length;
            });
        });
    },

    /**
     * Convert a template string into HTML DOM nodes
     * @param  {String} str The template string
     * @return {Node}       The template HTML
     */
    _stringToHTML(str) {
        const temp = document.createElement('div');
        temp.innerHTML = str;
        return temp;
    },

    _resetKeyboard() {
        this.elements = {
            main: null,
            langs: [],
            keysContainer: null,
            keys: [],
            textElement: null
        };

        this.properties = {
            value: "",
            capsLock: 0,
            altGr: false,
            accent: false,
            carretPosition: 0
        };

        if (document.querySelector('.keyboard')) {
            document.querySelector('.keyboard').remove();
        }
    },

    _createKeys(layout) {
        const fragment = document.createDocumentFragment();
        const keyLayout = charset[layout]
		

        keyLayout.forEach(key => {
            const keyElement = document.createElement("button");

            if (!key.newline) {
                // Add attributes/classes
                keyElement.setAttribute("type", "button");
                keyElement.classList.add("keyboard__key");

                switch (key.special) {
                    case "BACKSPACE":
                        keyElement.classList.add("keyboard__key");
                        keyElement.classList.add("backspace");

                        keyElement.dataset.special = true;

                        keyElement.addEventListener("click", () => {
                            const value = this.properties.value.substring(0, this.properties.carretPosition - 1) + this.properties.value.substring(this.properties.carretPosition, this.properties.value.length);

                            this.properties.carretPosition = this.properties.carretPosition !== 0 ? this.properties.carretPosition - 1 : 0;
                            this.properties.value = value;

                            this._triggerEvent("oninput");
                        });

                        break;

                    case "SHIFT":
                        keyElement.classList.add("keyboard__key", "keyboard__key--activatable");
                        keyElement.classList.add("shift");

                        keyElement.dataset.special = true;

                        keyElement.addEventListener("click", () => {
                            this.properties.capsLock = (this.properties.capsLock + 1) % 3;

                            this._toggleCapsLock();

                            keyElement.classList.toggle("keyboard__key--active", this.properties.capsLock === 2);
                        });

                        break;

                    case "ENTER":
                        keyElement.classList.add("keyboard__key");
                        keyElement.classList.add("enter");

                        keyElement.dataset.special = true;

                        keyElement.addEventListener("click", () => {
                            if (this.elements.textElement.nodeName === "INPUT") {
                                this.close();
                            }

                            const value = this.properties.value.substring(0, this.properties.carretPosition) + "\n" + this.properties.value.substring(this.properties.carretPosition)

                            this.properties.carretPosition++;
                            this.properties.value = value;

                            this._triggerEvent("oninput");
                        });

                        break;

                    case "ALTGR":
                        keyElement.classList.add("keyboard__key--wide", "keyboard__key--activatable");
                        keyElement.innerHTML = "Alt Gr";
                        keyElement.dataset.keyAltGr = true;

                        keyElement.dataset.special = true;

                        keyElement.addEventListener("click", () => {
                            this._toggleAltGr();

                            const accent = document.querySelector('[data-key-accent="true"]');

                            if (accent) {
                                accent.disabled = this.properties.altGr;
                            }

                            keyElement.classList.toggle("keyboard__key--active", this.properties.altGr);
                        });

                        break;

                    case "FLAG":
                        keyElement.classList.add("keyboard__key");
                        keyElement.innerHTML = `<div class="key__flag" style="background-image: url('${this.elements.root || ''}img/flags/${key.cls.toUpperCase()}.png')"></div>`;

                        keyElement.dataset.special = true;

                        keyElement.addEventListener("click", (e) => {
                            const selection = document.querySelector('.layout-type-container');
                            if (selection.style.display === 'block') {
                                selection.style.display = 'none';
                                return;
                            }

                            selection.style.display = 'block';

                            selection.style.left = (keyElement.offsetLeft - 100) + 'px'
                        });

                        break;

                    case "ACCENT":
                        keyElement.classList.add("keyboard__key", "keyboard__key--activatable");

                        keyElement.dataset.special = true;
                        keyElement.dataset.keyAccent = true;

                        keyElement.innerHTML = //html
                            `
                                <div class="key__container">
                                    <div class="key__sup disabled">${key.sup}</div>
                                    <div class="key__sub disabled">${key.sub}</div>
                                </div>
                            `;
                        keyElement.addEventListener("click", () => {
                            this._toggleAccent();

                            const altGr = document.querySelector('[data-key-alt-gr="true"]');

                            if (altGr) {
                                altGr.disabled = this.properties.accent !== 0;
                            }

                            if (this.properties.accent === 1) {
                                keyElement.querySelector('.key__sub').classList.remove("disabled");
                            }

                            if (this.properties.accent === 2) {
                                keyElement.querySelector('.key__sub').classList.add("disabled");
                                keyElement.querySelector('.key__sup').classList.remove("disabled");
                            }

                            if (this.properties.accent === 0) {
                                keyElement.querySelector('.key__sub').classList.add("disabled");
                                keyElement.querySelector('.key__sup').classList.add("disabled");
                            }

                            keyElement.classList.toggle("keyboard__key--active", this.properties.accent);
                        });

                        break;

                    case "SPACE":
                        keyElement.classList.add("keyboard__key--extra-wide");

                        keyElement.dataset.special = true;

                        keyElement.addEventListener("click", () => {
                            const SPACE = " ";
                            const value = this.properties.value.substring(0, this.properties.carretPosition) + SPACE + this.properties.value.substring(this.properties.carretPosition)

                            this.properties.carretPosition++;
                            this.properties.value = value;

                            this._triggerEvent("oninput");
                        });

                        break;

                    case "CONFIRM":
                        keyElement.classList.add("keyboard__key--wide", "keyboard__key--dark");
                        keyElement.classList.add("check");

                        keyElement.dataset.special = true;

                        keyElement.addEventListener("click", () => {
                            this.close();
                            this._triggerEvent("onclose");
                        });

                        break;

                    default:
                        let html = '';

                        if (key.sup && key.altgr && key.altgrsup) {
                            html = //html
                                `
                                <div data-has-accent="${key.accent ? true : false}" class="key__container left">
                                    <div class="key__sup disabled">${key.sup}</div>
                                    <div class="key__sup keyboard__key__altgrsup altgr">${key.altgrsup}</div>
                                    <div class="key__sub">${key.sub}</div>
                                    <div class="keyboard__key__altgr altgr">${key.altgr}</div>
                                </div>
                            `;
                        } else if (key.sup && key.altgr) {
                            html = //html
                                `
                                <div data-has-accent="${key.accent ? true : false}" class="key__container left">
                                    <div class="key__sup disabled">${key.sup}</div>
                                    <div class="key__sub">${key.sub}</div>
                                    <div class="keyboard__key__altgr altgr">${key.altgr}</div>
                                </div>
                            `;
                        } else if (key.sup) {
                            html = //html
                                `
                                    <div data-has-accent="${key.accent ? true : false}" class="key__container">
                                        <div class="key__sup disabled">${key.sup}</div>
                                        <div class="key__sub">${key.sub}</div>
                                    </div>
                                `;
                        } else {
                            html = //html
                                `
                                    <div data-has-accent="${key.accent ? true : false}" class="key__container">
                                        <div class="std">${key.sub}</div>
                                        <div class="keyboard__key__altgr altgr">${key.altgr || ''}</div>
                                    </div>
                                `;
                        }

                        keyElement.innerHTML = html;

                        keyElement.addEventListener("click", () => {
                            if (this.properties.altGr) {
                                if (key.altgrsup && this.properties.capsLock) {
                                    const fn = this.properties.capsLock ? 'toUpperCase' : 'toLowerCase';
                                    const value = this.properties.value.substring(0, this.properties.carretPosition) + key.altgrsup[fn]() + this.properties.value.substring(this.properties.carretPosition)

                                    this.properties.carretPosition++;
                                    this.properties.value = value;
                                } else if (key.altgr) {
                                    const fn = this.properties.capsLock ? 'toUpperCase' : 'toLowerCase';
                                    const value = this.properties.value.substring(0, this.properties.carretPosition) + key.altgr[fn]() + this.properties.value.substring(this.properties.carretPosition)

                                    this.properties.carretPosition++;
                                    this.properties.value = value;
                                }
                            } else if (this.properties.accent) {
                                if (key.accent) {
                                    const fn = this.properties.capsLock ? 'toUpperCase' : 'toLowerCase';
                                    const character = this.properties.accent === 1 ? key.a_1[fn]() : key.a_2[fn]();
                                    const value = this.properties.value.substring(0, this.properties.carretPosition) + character + this.properties.value.substring(this.properties.carretPosition)

                                    this.properties.carretPosition++;
                                    this.properties.value = value;
                                }
                            } else {
                                if (this.properties.capsLock && !key.sup) {
                                    const value = this.properties.value.substring(0, this.properties.carretPosition) + key.sub.toUpperCase() + this.properties.value.substring(this.properties.carretPosition)

                                    this.properties.carretPosition++;
                                    this.properties.value = value;
                                } else if (this.properties.capsLock && key.sup) {
                                    const value = this.properties.value.substring(0, this.properties.carretPosition) + key.sup + this.properties.value.substring(this.properties.carretPosition)

                                    this.properties.carretPosition++;
                                    this.properties.value = value;
                                } else {
                                    const value = this.properties.value.substring(0, this.properties.carretPosition) + key.sub + this.properties.value.substring(this.properties.carretPosition)

                                    this.properties.carretPosition++;
                                    this.properties.value = value;
                                }
                            }

                            if (this.properties.capsLock === 1)
                                this.properties.capsLock = 0;

                            this._toggleCapsLock();

                            this._triggerEvent("oninput");
                        });

                        break;
                }

                fragment.appendChild(keyElement);
            }

            if (key.newline) {
                fragment.appendChild(document.createElement("br"));
            }
        });

        return fragment;
    },

    _triggerEvent(handlerName) {
        if (typeof this.eventHandlers[handlerName] == "function") {
            this.eventHandlers[handlerName](this.properties.value);

            if (handlerName === 'oninput') {
                this.elements.main.querySelector('.text-reference .txt').innerHTML = this.properties.value;
            }
        }
    },

    _toggleAltGr() {
        this.properties.altGr = !this.properties.altGr;

        for (const key of this.elements.keys) {
            if (key.querySelector('.key__sup')) {
                key.querySelector('.key__sup').classList.toggle("altgr");
            }

            if (key.querySelector('.key__sub')) {
                key.querySelector('.key__sub').classList.toggle("altgr");
            }

            if (key.querySelector('.std')) {
                key.querySelector('.std').classList.toggle("altgr");
            }

            if (key.querySelector('.keyboard__key__altgr')) {
                if (!this.properties.capsLock && this.properties.altGr) key.querySelector('.keyboard__key__altgr').classList.remove("altgr");
                if (!this.properties.capsLock && this.properties.altGr) {
                    if (key.querySelector('.keyboard__key__altgrsup')) key.querySelector('.keyboard__key__altgrsup').classList.add("altgr");
                }
            }

            if (key.querySelector('.keyboard__key__altgrsup')) {
                if (this.properties.capsLock && this.properties.altGr) {
                    if (key.querySelector('.keyboard__key__altgr')) key.querySelector('.keyboard__key__altgr').classList.add("altgr");
                }
                if (this.properties.capsLock && this.properties.altGr) key.querySelector('.keyboard__key__altgrsup').classList.remove("altgr");
            }

            if (!this.properties.altGr) {
                if (key.querySelector('.keyboard__key__altgr')) key.querySelector('.keyboard__key__altgr').classList.add("altgr");
                if (key.querySelector('.keyboard__key__altgrsup')) key.querySelector('.keyboard__key__altgrsup').classList.add("altgr");
            }
        }
    },

    _toggleCapsLock() {
        for (const key of this.elements.keys) {
            if (key.dataset.special) {
                continue;
            }

            if (key.querySelector('.std')) {
                key.querySelector('.std').innerHTML = this.properties.capsLock > 0 ? key.querySelector('.std').innerHTML.toUpperCase() : key.querySelector('.std').innerHTML.toLowerCase();
            }

            if (this.properties.capsLock > 0) {
                if (key.querySelector('.key__sup')) key.querySelector('.key__sup').classList.remove("disabled");
                if (key.querySelector('.key__sub')) key.querySelector('.key__sub').classList.add("disabled");
            }

            if (this.properties.capsLock === 0) {
                if (key.querySelector('.key__sup')) key.querySelector('.key__sup').classList.add("disabled");
                if (key.querySelector('.key__sub')) key.querySelector('.key__sub').classList.remove("disabled");
            }


            if (key.querySelector('.keyboard__key__altgr')) {
                if (!this.properties.capsLock && this.properties.altGr) key.querySelector('.keyboard__key__altgr').classList.remove("altgr");
                if (!this.properties.capsLock && this.properties.altGr) {
                    if (key.querySelector('.keyboard__key__altgrsup')) key.querySelector('.keyboard__key__altgrsup').classList.add("altgr");
                }
            }

            if (key.querySelector('.keyboard__key__altgrsup')) {
                if (this.properties.capsLock && this.properties.altGr) {
                    if (key.querySelector('.keyboard__key__altgr')) key.querySelector('.keyboard__key__altgr').classList.add("altgr");
                }
                if (this.properties.capsLock && this.properties.altGr) key.querySelector('.keyboard__key__altgrsup').classList.remove("altgr");
            }

            if (!this.properties.altGr) {
                if (key.querySelector('.keyboard__key__altgr')) key.querySelector('.keyboard__key__altgr').classList.add("altgr");
                if (key.querySelector('.keyboard__key__altgrsup')) key.querySelector('.keyboard__key__altgrsup').classList.add("altgr");
            }
        }
    },

    _toggleAccent() {
        this.properties.accent = (this.properties.accent + 1) % 3;

        document.querySelectorAll('[data-has-accent="false"]').forEach(key => {
            key.classList[this.properties.accent ? 'add' : 'remove']('accent');
        });
    },

    open(initialValue, oninput, onclose) {
        this.properties.value = initialValue || "";
        this.eventHandlers.oninput = oninput;
        this.eventHandlers.onclose = onclose;
        this.elements.main.classList.remove("keyboard--hidden");

        this.elements.main.querySelector('.text-reference .txt').innerHTML = this.properties.value;
    },

    close() {
        this.properties.value = "";
        this.eventHandlers.oninput = oninput;
        this.eventHandlers.onclose = onclose;
        this.elements.main.classList.add("keyboard--hidden");

        this.elements.main.querySelector('.text-reference .txt').innerHTML = "";
    }
};

const charset = {
    EN: [
        { sub: "`", sup: "¬", altgr: "¦" },
        { sub: "1", sup: "!", altgr: null },
        { sub: "2", sup: '"', altgr: null },
        { sub: "3", sup: "£", altgr: null },
        { sub: "4", sup: "$", altgr: "€" },
        { sub: "5", sup: "%", altgr: null },
        { sub: "6", sup: "^", altgr: null },
        { sub: "7", sup: "&", altgr: null },
        { sub: "8", sup: "*", altgr: null },
        { sub: "9", sup: "(", altgr: null },
        { sub: "0", sup: ")", altgr: null },
        { sub: "-", sup: "_", altgr: null },
        { sub: "=", sup: "+", altgr: null },
        { special: 'BACKSPACE' },
        { newline: true },
        // --------------------------------
        { sub: "q", sup: null, altgr: null },
        { sub: "w", sup: null, altgr: null },
        { sub: "e", sup: null, altgr: null },
        { sub: "r", sup: null, altgr: null },
        { sub: "t", sup: null, altgr: null },
        { sub: "y", sup: null, altgr: null },
        { sub: "u", sup: null, altgr: null },
        { sub: "i", sup: null, altgr: null },
        { sub: "o", sup: null, altgr: null },
        { sub: "p", sup: null, altgr: null },
        { sub: "[", sup: "{", altgr: null },
        { sub: "]", sup: "}", altgr: null },
        { special: 'ENTER' },
        { newline: true },
        // ---------------------------------
        { special: 'SHIFT' },
        { sub: "a", sup: null, altgr: null },
        { sub: "s", sup: null, altgr: null },
        { sub: "d", sup: null, altgr: null },
        { sub: "f", sup: null, altgr: null },
        { sub: "g", sup: null, altgr: null },
        { sub: "h", sup: null, altgr: null },
        { sub: "j", sup: null, altgr: null },
        { sub: "k", sup: null, altgr: null },
        { sub: "l", sup: null, altgr: null },
        { sub: ";", sup: ":", altgr: null },
        { sub: "'", sup: "@", altgr: null },
        { sub: "#", sup: "~", altgr: null },
        { newline: true },
        // ---------------------------------
        { sub: "\\", sup: "|", altgr: null },
        { sub: "z", sup: null, altgr: null },
        { sub: "x", sup: null, altgr: null },
        { sub: "c", sup: null, altgr: null },
        { sub: "v", sup: null, altgr: null },
        { sub: "b", sup: null, altgr: null },
        { sub: "n", sup: null, altgr: null },
        { sub: "m", sup: null, altgr: null },
        { sub: ",", sup: "<", altgr: null },
        { sub: ".", sup: ">", altgr: null },
        { sub: "/", sup: "?", altgr: null },
        { newline: true },
        // ---------------------------------
        { special: 'CONFIRM' },
        { special: 'SPACE' },
        { special: 'ALTGR' },
        { special: 'FLAG', cls: 'EN' },
    ],
    IT: [
        { sub: "\\", sup: "|" },
        { sub: "1", sup: "!" },
        { sub: "2", sup: '"' },
        { sub: "3", sup: "£" },
        { sub: "4", sup: "$" },
        { sub: "5", sup: "%", altgr: "€" },
        { sub: "6", sup: "&" },
        { sub: "7", sup: "/" },
        { sub: "8", sup: "(" },
        { sub: "9", sup: ")" },
        { sub: "0", sup: "=" },
        { sub: "'", sup: "?" },
        { sub: "ì", sup: "^" },
        { special: 'BACKSPACE' },
        { newline: true },
        // --------------------------------
        { sub: "q", sup: null, altgr: null },
        { sub: "w", sup: null, altgr: null },
        { sub: "e", sup: null, altgr: "€" },
        { sub: "r", sup: null, altgr: null },
        { sub: "t", sup: null, altgr: null },
        { sub: "y", sup: null, altgr: null },
        { sub: "u", sup: null, altgr: null },
        { sub: "i", sup: null, altgr: null },
        { sub: "o", sup: null, altgr: null },
        { sub: "p", sup: null, altgr: null },
        { sub: "è", sup: "é", altgr: "[", altgrsup: '{' },
        { sub: "+", sup: "*", altgr: "]", altgrsup: '}' },
        { special: 'ENTER' },
        { newline: true },
        // ---------------------------------
        { special: 'SHIFT' },
        { sub: "a", sup: null, altgr: null },
        { sub: "s", sup: null, altgr: null },
        { sub: "d", sup: null, altgr: null },
        { sub: "f", sup: null, altgr: null },
        { sub: "g", sup: null, altgr: null },
        { sub: "h", sup: null, altgr: null },
        { sub: "j", sup: null, altgr: null },
        { sub: "k", sup: null, altgr: null },
        { sub: "l", sup: null, altgr: null },
        { sub: "ò", sup: "ç", altgr: "@" },
        { sub: "à", sup: "°", altgr: "#" },
        { sub: "ù", sup: "§", altgr: null },
        { newline: true },
        // ---------------------------------
        { sub: "<", sup: ">", altgr: null },
        { sub: "z", sup: null, altgr: null },
        { sub: "x", sup: null, altgr: null },
        { sub: "c", sup: null, altgr: null },
        { sub: "v", sup: null, altgr: null },
        { sub: "b", sup: null, altgr: null },
        { sub: "n", sup: null, altgr: null },
        { sub: "m", sup: null, altgr: null },
        { sub: ",", sup: ";", altgr: null },
        { sub: ".", sup: ":", altgr: null },
        { sub: "-", sup: "_", altgr: null },
        { newline: true },
        // ---------------------------------
        { special: 'CONFIRM' },
        { special: 'SPACE' },
        { special: 'ALTGR' },
        { special: 'FLAG', cls: 'IT' },
    ],
    FR: [
        { sub: "&", sup: "1" },
        { sub: "é", sup: "2", altgr: "~" },
        { sub: '"', sup: "3", altgr: "#" },
        { sub: "'", sup: "4", altgr: "{" },
        { sub: "(", sup: "5", altgr: "[" },
        { sub: "-", sup: "6", altgr: "|" },
        { sub: "è", sup: "7", altgr: "`" },
        { sub: "_", sup: "8", altgr: "\\" },
        { sub: "ç", sup: "9", altgr: "^" },
        { sub: "à", sup: "0", altgr: "@" },
        { sub: ")", sup: "°", altgr: "]" },
        { sub: "=", sup: "+", altgr: "}" },
        { special: 'BACKSPACE' },
        { newline: true },
        // --------------------------------
        { sub: "a", sup: null, altgr: "æ", accent: true, a_1: "â", a_2: "ä" },
        { sub: "z", sup: null, altgr: null },
        { sub: "e", sup: null, altgr: "€", accent: true, a_1: "ê", a_2: "ë" },
        { sub: "r", sup: null, altgr: null },
        { sub: "t", sup: null, altgr: null },
        { sub: "y", sup: null, altgr: null },
        { sub: "u", sup: null, altgr: null, accent: true, a_1: "û", a_2: "ü" },
        { sub: "i", sup: null, altgr: null, accent: true, a_1: "î", a_2: "ï" },
        { sub: "o", sup: null, altgr: "œ", accent: true, a_1: "ô", a_2: "ö" },
        { sub: "p", sup: null, altgr: null },
        { sub: "£", sup: "$", altgr: null },
        { special: 'ENTER' },
        { newline: true },
        // ---------------------------------
        { special: 'SHIFT' },
        { sub: "q", sup: null, altgr: null },
        { sub: "s", sup: null, altgr: null },
        { sub: "d", sup: null, altgr: null },
        { sub: "f", sup: null, altgr: null },
        { sub: "g", sup: null, altgr: null },
        { sub: "h", sup: null, altgr: null },
        { sub: "j", sup: null, altgr: null },
        { sub: "k", sup: null, altgr: null },
        { sub: "l", sup: null, altgr: null },
        { sub: "m", sup: null, altgr: null },
        { sub: "ù", sup: "%", altgr: null },
        { sub: "*", sup: "µ", altgr: null },
        { newline: true },
        // ---------------------------------
        { sub: "<", sup: ">", altgr: null },
        { sub: "w", sup: null, altgr: null },
        { sub: "x", sup: null, altgr: null },
        { sub: "c", sup: null, altgr: null },
        { sub: "v", sup: null, altgr: null },
        { sub: "b", sup: null, altgr: null },
        { sub: "n", sup: null, altgr: null },
        { sub: ",", sup: "?", altgr: null },
        { sub: ";", sup: ".", altgr: null },
        { sub: ":", sup: "/", altgr: null },
        { sub: "ù", sup: "%", altgr: null },
        { sub: "!", sup: "§", altgr: null },
        { newline: true },
        // ---------------------------------
        { special: 'CONFIRM' },
        { special: 'SPACE' },
        { special: 'ALTGR' },
        { special: 'ACCENT', sub: "^", sup: "¨", altgr: null },
        { special: 'FLAG', cls: 'FR' },
    ]
}
