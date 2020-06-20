function cin(question, suggestion){
    let inp = prompt(question, suggestion);
    return inp;
}


class Interface{
    constructor(){
        this.__init_connection();

        this.__att_clicked = -1;
        this.__prev_clicked = -1;

        this.__content =  document.createElement("div");
        document.body.appendChild(this.__content);
        let C = cols_div.get_equispaced(this.__content ,2 );

        this.__left_menu = C.at(0);
        this.__left_menu.setAttribute("class", "panel");
        this.__left_menu.style.width =  (100.0 * 1 / columns) + "%"; 

        this.__network = C.at(1);
        
        this.__update(-1);

        let this_ref = this;
        this.__buttons = [];
        function create_button(image, descr){
            let d = document.createElement("div");
            d.style.height = H_panel + "px";
            put_button(d, image, descr);
            return d;
        }
        this.__buttons.push(create_button("./img_GUI/Query/O.svg" ,  "Set observed value"));
        this.__buttons.push(create_button("./img_GUI/Query/I.svg" ,  "Compute marginals"));
        this.__buttons.push(create_button("./img_GUI/Modify/P_m.svg" ,  "Add unary potential"));
        this.__buttons.push(document.createElement("div"));

        this.__buttons.push(create_button("./img_GUI/Modify/P_b.svg" ,  "Add binary potential"));
        
        this.__vertical_P = new popup_vertical_menu(this.__buttons[3], H_panel, create_button("./img_GUI/Modify/P_b.svg" ,  "Add binary potential"));
        
        let binary_menu_C_S = [create_button("./img_GUI/Modify/P_b_T.svg" ,  "Add correlating potential") , create_button("./img_GUI/Modify/P_b_F.svg" ,  "Add anti-correlating potential"), create_button("./img_GUI/Modify/P_b_file.svg" ,  "Import binary shape from file")];
        let binary_menu_S = [binary_menu_C_S[2]];
        this.__vertical_P.add_menu(binary_menu_C_S);
        this.__vertical_P.add_menu(binary_menu_S);

        this.__buttons[0].addEventListener("click", function(){ 
            let val = cin("Set the value for the observation", "0");
            if(val == null) return;
            this_ref.send_command("O",["v","n"],[this_ref.__names_sizes[this_ref.__att_clicked].N,val]); 
        });
        this.__buttons[1].addEventListener("click", function(){ 
            this_ref.send_command("I",["v"],[this_ref.__names_sizes[this_ref.__att_clicked].N]); 
        });
        this.__buttons[2].addEventListener("click", function(){ 
            let w = cin("Set the weight (0 for simple shape)", "0");
            if(w == null) return;
            if(w == 0) this_ref.send_download("P",["v", "s"], [this_ref.__names_sizes[this_ref.__att_clicked], "%download"]); 
            else  this_ref.send_download("P",["v", "s", "w"], [this_ref.__names_sizes[this_ref.__att_clicked], "%download", w]); 
        });

        binary_menu_C_S[0].addEventListener("click", function(){ 
            let w = cin("Set the weight (0 for simple shape)", "0");
            if(w == null) return;
            if(w == 0) this_ref.send_command("P",["v", "v", "c"], 
            [this_ref.__names_sizes[this_ref.__att_clicked].N, this_ref.__names_sizes[this_ref.__prev_clicked].N, "T"]); 
            else this_ref.send_command("P",["v", "v", "c", "w"], 
            [this_ref.__names_sizes[this_ref.__att_clicked].N, this_ref.__names_sizes[this_ref.__prev_clicked].N, "T", w]); 
        });
        binary_menu_C_S[1].addEventListener("click", function(){ 
            let w = cin("Set the weight (0 for simple shape)", "0");
            if(w == null) return;
            if(w == 0) this_ref.send_command("P",["v", "v", "c"], 
            [this_ref.__names_sizes[this_ref.__att_clicked].N, this_ref.__names_sizes[this_ref.__prev_clicked].N, "F"]); 
            else this_ref.send_command("P",["v", "v", "c", "w"], 
            [this_ref.__names_sizes[this_ref.__att_clicked].N, this_ref.__names_sizes[this_ref.__prev_clicked].N, "F", w]); 
        });
        binary_menu_C_S[2].addEventListener("click", function(){ 
            let w = cin("Set the weight (0 for simple shape)", "0");
            if(w == null) return;
            if(w == 0) this_ref.send_download("P",["v", "v", "s"], 
            [this_ref.__names_sizes[this_ref.__att_clicked].N, this_ref.__names_sizes[this_ref.__prev_clicked].N, "%download"]); 
            else this_ref.send_download("P",["v", "v", "s", "w"], 
            [this_ref.__names_sizes[this_ref.__att_clicked].N, this_ref.__names_sizes[this_ref.__prev_clicked].N, "%download", w]); 
        });


        this.__Catcher =  new file_catcher();
    }

    __init_connection(){
        this.__server_key = '-';
		let handshake_req = new XMLHttpRequest();
		handshake_req.open('GET', 'http://localhost:8001', false);
		handshake_req.setRequestHeader('Accept', this.__server_key + '_hello');
		handshake_req.send();
		this.__server_key = handshake_req.response;
        console.log("server key: " + this.__server_key);
        
        this.__structure = null;
        this.__names_sizes = [];
        this.__marginals = [];
    }

    send_command(command_symbol, field_names = [], field_values = []){
		var upd_req = new XMLHttpRequest();
		let this_ref = this;
		upd_req.onload = function() { 
			let x =  upd_req.response;
				if(x == 'null'){
					return;
				}
				else {
					this_ref.__update_network(x);
				}
		}
		
        upd_req.open('GET', 'http://localhost:8001', true);
        let v = this.__server_key + '_' + command_symbol;
        for(let i = 0; i<field_names.length; i++){
            v += "$" + field_names[i];
            v += "$" + field_values[i];
        }
        console.log(v);
		upd_req.setRequestHeader('Accept', v);
		upd_req.send();
    }
    
    send_download(command_symbol, field_names, field_values){

        let this_ref = this;
        let dwld = function(){
            this_ref.send_command(command_symbol, field_names, field_values);
        };
        this.__Catcher.download_file("__temp_download__.txt", dwld);

    }

    __update(id){ //pass -1 when de-selected
        if(id == -1){
            this.__prev_clicked = this.__att_clicked;

            this.__network.style.width = "100%";
            this.__content.innerHTML = "";
            this.__content.appendChild(this.__network);

        }
        else{
            this.__att_clicked = id;
            if(this.__prev_clicked == id) this.__prev_clicked = -1;
            
            this.__content.innerHTML = "";
            this.__network.style.width =  100*(1-1 / columns) + "%";
            this.__content.innerHTML = "";
            this.__content.appendChild(this.__left_menu);
            this.__content.appendChild(this.__network);


            this.__left_menu.innerHTML = "";

            let this_ref = this;
            function add_info(testo){
                let info = document.createElement("div");
                info.style.height  = H_panel + "px";
                let txt = document.createElement("font");
                txt.setAttribute("color", "white");
                txt.innerHTML = testo;
                info.appendChild(txt);
                this_ref.__left_menu.appendChild(info);

            };
            add_info("Name: " + this.__names_sizes[id].N);
            add_info("Size: " + this.__names_sizes[id].S);

            //O P_unary
            if(id < this.__marginals.length) {
                this.__left_menu.appendChild(this.__buttons[0]);
                this.__left_menu.appendChild(this.__buttons[2]);
            }
            // P_binary
            if(this.__prev_clicked != -1) {
                if(this.__names_sizes[this.__att_clicked].S ==  this.__names_sizes[this.__prev_clicked].S) this.__vertical_P.select_menu(0);
                else this.__vertical_P.select_menu(1);
                this.__left_menu.appendChild(this.__buttons[3]);
            }
            //I
            if(id < this.__marginals.length) {
                if(this.__marginals[id].length == 0) this.__left_menu.appendChild(this.__buttons[1]);
                else this.__plot_marginal(this.__marginals[id]);
            }
            
        }
    }

    __update_network(new_net){

		if (this.__structure !== null) {
			this.__structure.destroy();
			this.__structure = null;
        }
        
		let options = {
			// layout:{randomSeed:2},
			// nodes: {font: {color: '#ffffff'}},
			interaction:{hover:true, selectable: true}	
        };
        
        let temp_JSON = JSON.parse(new_net);

        let temp_net_JSON = {nodes : 0, edges:0};
        temp_net_JSON.nodes = temp_JSON.nodes;
        temp_net_JSON.edges = temp_JSON.edges;

        this.__structure = new vis.Network(this.__network, temp_net_JSON, options);

        this.__marginals = temp_JSON.marginals;
        this.__names_sizes = temp_JSON.names;
		
		let this_ref = this;
		this.__structure.on("click", function (params) {
            params.event = "[original event]";
            let id_node = parseInt(params.nodes[0]);
            if(id_node < this_ref.__names_sizes.length  ) this_ref.__update(id_node);
            else this_ref.__update(-1);
		});
		
		this.__structure.on("deselectNode", function (params) {
			this_ref.__update(-1);
        });
        	            
    }     	
    
    __plot_marginal(vals){
        let Canvas =  document.createElement("div");
        document.body.appendChild(Canvas);
        Canvas.style.height = "150px";        
        draw_histograms(Canvas , vals, "#81a600");
        this.__left_menu.appendChild(Canvas);
    }
}
