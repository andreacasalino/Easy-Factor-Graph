var H_panel = 45;

function get_image(x, y, W, H, relative_path){
    let img_temp = document.createElementNS("http://www.w3.org/2000/svg","image");
    img_temp.setAttributeNS(null, "x", x);
    img_temp.setAttributeNS(null, "y", y);
    img_temp.setAttributeNS(null, "width", W);
    img_temp.setAttributeNS(null, "height", H);
    img_temp.setAttributeNS(null, "preserveAspectRatio", "none");
    img_temp.setAttributeNS(null, "href", relative_path);
    return img_temp;
}

function get_SVG_canvas(){
    let SVG_frame  = document.createElementNS("http://www.w3.org/2000/svg","svg");
    SVG_frame.setAttributeNS(null, "width" , "100%");
    SVG_frame.setAttributeNS(null, "height" , "100%");
    SVG_frame.setAttributeNS(null, "viewBox", "0 0 100 100");
    return SVG_frame;
}

function put_button(wrapping, image, description){

    let center_div = document.createElement("div");
    center_div.setAttribute("class", "borded");
    put_in_the_middle2(wrapping, center_div, [0.02,0.2]);
    let center_div_boxes = new cols_div(center_div , [3,7]);
    let cnvs = get_SVG_canvas();
    cnvs.appendChild(get_image(0,0,100,100,image));
    center_div_boxes.at(0).appendChild(cnvs);
    let descr = document.createElement("font");
    descr.setAttribute("color", "white");
    descr.innerHTML = description;
    center_div_boxes.at(1).appendChild(descr);

    center_div.style.cursor = "pointer";
    center_div.addEventListener("mouseover" , function(){ center_div.style.opacity = 0.5; });
    center_div.addEventListener("mouseout" , function(){ center_div.style.opacity = 1; });

}

function get_panel(images, descriptions){

    if(images.length != descriptions.length) return null;

    let n = images.length;
    let panel = document.createElement("div");
    panel.style.height = H_panel +"px";
    let boxes = cols_div.get_equispaced(panel, columns);
    for(let i=0; i<n; i++){
        put_button(boxes.at(i), images[i], descriptions[i]);
    }
    return panel;

}

