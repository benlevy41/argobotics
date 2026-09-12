const CONTROL_RATE = 50; //ms

class Key {
    constructor(key){
        this.key = key;
        this.is_active = false;
    }

    set(){
        if (this.is_active === false) {
            this.is_active = true;
        }
    }

    reset(){
        if (this.is_active){
            this.is_active = false;
        }
    }
}

class Control {
    constructor(pos, neg, command, step){
        this.pos = pos;
        this.neg = neg;
        this.command = command;
        this.step = step;
        this.val = 0;
    }

    set(key){
        if (key === this.pos) {
            this.val += this.step;
        } else if (key === this.neg) {
            this.val -= this.step;
        } else {
            return;
        }

        this.val = Math.max(-99, Math.min(99, this.val));

        return this.val;
    }

    reset(key){
        return;
    }
}


class MomentControl extends Control {
    constructor(pos, neg, command, step){
        super(new Key(pos), new Key(neg), command, step);
    }

    set(key){
        if (key === this.pos.key && this.pos.is_active === false ) {
            this.neg.reset();
            this.pos.set();
            this.val += this.step;
        } else if (key === this.neg.key && this.neg.is_active === false ) {
            this.neg.set();
            this.pos.reset();
            this.val -= this.step;
        } else {
            return;
        }

        this.val = Math.max(-99, Math.min(99, this.val));

        return this.val;
    }

    reset(key){
        if (key === this.pos.key && this.pos.is_active) {
            this.pos.reset();
            this.val = 0;
            return this.val;
        }

        if (key === this.neg.key && this.neg.is_active) {
            this.neg.reset();
            this.val = 0;
            return this.val;
        }

        return;
    }
}

class KeyController {
    constructor(){
        this.state = new Map([
            ['t', 0],
            ['y', 0],
            ['h', 0],
            ['p', 0],
            ['l', 0],
        ])

        this.macros = [
            ' ',    //allStop
            '0'     //centerCam
        ]

        this.controls = [
            new MomentControl('w','s','t',33),
            new MomentControl('a','d','y',33),
            new Control('e','q','h',3),
            new Control(']','[','p',3),
            new Control('=','-','l',3)
        ];
    }

    allStop(){
        this.state.set('t', 0);
        this.state.set('y', 0);
        this.state.set('h', 0);
    }

    centerCam(){
        this.state.set('p', 0);
        this.state.set('l', 0);
    }
    
    handleKeyDown(input){
        if (this.macros.includes(input)){
            switch(input) {
                case ' ':
                    this.allStop();
                    break;
                case '0':
                    this.centerCam();
                    break;
            }
        }
        for (const control of this.controls){
            const val = control.set(input);
            if (val !== undefined){
                this.state.set(control.command, val);
                break;
            }
        }
    }

    handleKeyUp(input){
        for (const control of this.controls){
            const val = control.reset(input);
            if (val !== undefined){
                this.state.set(control.command, val);
                break;
            }
        }
    }

    read_state(){
        return [...this.state]
            .map(([key, value]) => `${key}:${value}`)
            .join(',');
    }

    get throttle(){
        return this.state.get('t');
    }

    get yaw(){
        return this.state.get('y');
    }

    get heave(){
        return this.state.get('h');
    }

    get pan(){
        return this.state.get('p');
    }

    get tilt(){
        return this.state.get('l');
    }
}

const socket = new WebSocket(`ws://${window.location.hostname}:8765`);
const connStatus = document.getElementById("connStatus");
const control = new KeyController();


socket.addEventListener("open", () => {
    connStatus.textContent = "Connected";
    console.log("WebSocket connected");
});

socket.addEventListener("close", () => {
    connStatus.textContent = "Disconnected";
    console.log("WebSocket disconnected");
});

socket.addEventListener("error", (error) => {
    console.error("WebSocket error:", error);
});

socket.addEventListener("message", (event) => {
    console.log("Received from server:", event.data);
});

function updateStateUi() {
    document.getElementById("state-t").textContent = control.throttle;
    document.getElementById("state-y").textContent = control.yaw;
    document.getElementById("state-h").textContent = control.heave;
    document.getElementById("state-p").textContent = control.pan;
    document.getElementById("state-l").textContent = control.tilt;
}

function updateSocketUi(is_conn){
    let text = ''
    if (is_conn){
        text = "Connected";
    } else {
       text = "Disconnected";
    }
    document.getElementById("wsStatus").textContent = text;
}

function sendCommand(command) {
    if (socket.readyState === WebSocket.OPEN) {
        socket.send(command);        
        console.log("sent:", command);
        updateSocketUi(true);
        updateStateUi();
    } else {
        updateSocketUi(false);
        console.warn("WebSocket is not connected");
    }
}

document.querySelectorAll("[data-key]").forEach(button => { //TODO: this should operate through the KeyController
    button.addEventListener("click", () => {
        control.handleKeyDown(button.dataset.key);
    });
});

setInterval(() => {
    const state  = control.read_state();
    sendCommand(sendCommand(state));
}, CONTROL_RATE);

document.addEventListener("keydown", (event) => {
    event.preventDefault();
    control.handleKeyDown(event.key.toLowerCase());
});

document.addEventListener("keyup", (event) => {
    event.preventDefault();
    control.handleKeyUp(event.key.toLowerCase());
});
