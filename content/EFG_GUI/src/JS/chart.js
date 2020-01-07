function draw_histograms(wrapping_div, vals, color="black"){

    let vals_sum = 0;
    for(let i=0; i<vals.length; i++){
        vals_sum += vals[i];
    }
    if(vals_sum >= 1e-3){
        for(let i=0; i<vals.length; i++){
            vals[i] = vals[i] / vals_sum;
        }
    }

    function get_rectangle(x, y, W, H){
        let rect_temp = document.createElementNS("http://www.w3.org/2000/svg","rect");
        rect_temp.setAttributeNS(null, "x" , x);
        rect_temp.setAttributeNS(null, "y" , y);
        rect_temp.setAttributeNS(null, "width" , W);
        rect_temp.setAttributeNS(null, "height" , H);
        rect_temp.setAttributeNS(null, "fill" , color);
        return rect_temp;
    }
    
    function get_text(x, y, content){
        let text_temp = document.createElementNS("http://www.w3.org/2000/svg","text");
        text_temp.setAttributeNS(null, "x" , x);
        text_temp.setAttributeNS(null, "y" , y);
        text_temp.setAttributeNS(null, "fill" , color);
        text_temp.setAttributeNS(null, "class" , "small");
        text_temp.innerHTML = content;
        return text_temp;
    }

    let margin = 2;
    let H_numb = 11;
    let H_val = 100 - H_numb;

    let canvas = get_SVG_canvas();
    wrapping_div.appendChild(canvas);

    let N_vals = vals.length;
    let delta_x = 100.0 / N_vals;

    let temp;
    for(let i=0; i<N_vals; i++){
        temp = get_text(delta_x*i + delta_x * 0.5 - margin, 100, i);
        canvas.appendChild(temp);

        temp = get_rectangle(delta_x*i + margin ,H_val * (1.0 - vals[i]) ,delta_x - (2*margin) , H_val * vals[i]);
        canvas.appendChild(temp);

        let VV = 100.0 * vals[i];
        temp.addEventListener("click", function(){
            alert(VV);
        });
    }

}