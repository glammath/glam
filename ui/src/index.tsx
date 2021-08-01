import './index.css';
import "./GlamCore"
import ReactDOM from "react-dom";
import {GraphController} from "./components/GraphController";
import React from "react";

window.addEventListener("nativeInit", () => {
    ReactDOM.render(<GraphController />,
        document.querySelector(".glam-app-container")
    );
})
