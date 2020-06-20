class popup_menu{
    constructor(popup_div, H_popup){
        this.__drivers = [];
        this.__H = H_popup;
        this.__div = popup_div;
        this.__reset();
    }

    add_driver(driver_div, to_show_div){
        let temp = {
            'driver' : driver_div,
            'to_show' : to_show_div
        }
        this.__drivers.push(temp);

        let this_ref = this;
        driver_div.addEventListener("click", function(e){
            let calling_driver = e.currentTarget;

            if(this_ref.__last_activated === null){
                this_ref.__div.style.height = this_ref.__H + "px";
                this_ref.__div.appendChild(this_ref.__get_to_show(calling_driver));
                this_ref.__last_activated = calling_driver;
            }
            else{
                if(calling_driver == this_ref.__last_activated){
                    this_ref.__reset();
                    this_ref.__last_activated = null;
                }
                else{
                    this_ref.__div.innerHTML = "";
                    this_ref.__div.appendChild(this_ref.__get_to_show(calling_driver));
                    this_ref.__last_activated = calling_driver;
                }
            }
        });

        this.__reset();
    }

    __reset(){
        this.__div.innerHTML = "";
        this.__div.style.height = "0px";
        this.__last_activated = null;
    }

    __get_to_show(calling_driver){

        for(let i =0; i<this.__drivers.length; i++){
            if(this.__drivers[i].driver == calling_driver){
                return this.__drivers[i].to_show;
            }
        }
        return null; //this is a bug

    }

}

class popup_vertical_menu{
    constructor(popup_div, H_popup, initial_div){
        this.__wrapping = popup_div;
        this.__elements = [];
        this.__H = H_popup;

        this.__initial = initial_div;
        this.__wrapping.innerHTML = "";
        this.__wrapping.style.height = this.__H + "px";
        this.__wrapping.appendChild(this.__initial);

        this.__state = 0;
        this.__list_to_show = 0;

        let this_ref =this;
        this.__initial.addEventListener("click", function(){
            this_ref.__update();
        });
    }

    select_menu(menu_count){
        this.__list_to_show = menu_count;
        this.__state = 1;
        this.__update();
    }

    add_menu(elements){
        for(let i=0; i<elements.length; i++){
            elements[i].style.height =  this.__H + "px";
        }
        this.__elements.push(elements);
        this.__state = 1;
        this.__update();
    }

    __update(){
        if(this.__state == 0){
            this.__wrapping.style.height = this.__H * (1 + this.__elements[this.__list_to_show].length) + "px";
            for(let i=0; i<this.__elements[this.__list_to_show].length; i++){
                this.__wrapping.appendChild(this.__elements[this.__list_to_show][i]);
            }
            this.__state = 1;
        }
        else{
            this.__wrapping.style.height = this.__H  + "px";
            this.__wrapping.innerHTML = "";
            this.__wrapping.appendChild(this.__initial);
            this.__state = 0;
        }
    }
}
