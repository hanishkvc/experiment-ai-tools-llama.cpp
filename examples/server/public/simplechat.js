// @ts-check

class SimpleChat {

    constructor() {
        this.xchat = /** @type {{role: string, content: string}[]} */([]);
    }

    /**
     * Add an entry into xchat
     * @param {string} role
     * @param {string} content
     */
    add(role, content) {
        this.xchat.push( {role: role, content: content} );
    }

    /**
     * Show the contents in the specified div
     * @param {HTMLDivElement} div 
     * @param {boolean} bClear 
     */
    show(div, bClear=true) {
        if (bClear) {
            div.replaceChildren();
        }
        for(const x of this.xchat) {
            let entry = document.createElement("p");
            entry.innerText = `${x.role}: ${x.content}`;
            div.appendChild(entry);
        }
    }

}

