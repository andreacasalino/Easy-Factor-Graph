class file_catcher{
    constructor(){
        this.__ghost_input = document.createElement("input");
        this.__ghost_input.setAttribute("type", "file");
        this.__callback = null;

        let this_ref  = this;
        this.__ghost_input.addEventListener("change", function(evt){
            for(let i=0; i<evt.target.files.length; i++){
                let f = evt.target.files[i]; 
                if (f) {
                    let r = new FileReader();
                    r.onload = function(e) { 
                        this_ref.__callback( e.target.result);
                    }
                    r.readAsText(f);
                } else { 
                    alert("Failed to load file");
                }
            }
        });
    }

    get_file_content(content_callback){
        this.__callback = content_callback;
        this.__ghost_input.click();
    }

    download_file(file_name = "__temp_download__.txt", dowload_callback = null){
        let dwld_fnct = function(file_content){
            let element = document.createElement('a');
            element.setAttribute('href', 'data:text/plain;charset=utf-8, ' + encodeURIComponent(file_content));   
            element.setAttribute('download', file_name);
            element.click();

            if(dowload_callback !== null) dowload_callback();
        };
        this.get_file_content(dwld_fnct);
    }
}