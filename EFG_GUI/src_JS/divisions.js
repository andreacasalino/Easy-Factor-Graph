function get_ones(numb_of_elements){
    let temp = [];
    for(let i=0; i<numb_of_elements;i++){
        temp.push(1);
    }
    return temp;
}

class array_of_div {
    constructor(containing_div, cells_ripartitions, prctg_or_pixel = 1){
        this.wrapping_div = containing_div;
        this.wrapping_div.innerHTML = "";
        this.cells=[];
        this.Sizes=[];

        if(prctg_or_pixel == 1){
            let summation = 0;
            for(let i = 0; i< cells_ripartitions.length; i++){
                summation = summation + cells_ripartitions[i];
            }
            for(let i = 0; i< cells_ripartitions.length; i++){
                this.Sizes.push(Math.round( 100 * cells_ripartitions[i] / summation * 100) / 100); 
            }
        }
        else{
            this.Sizes = cells_ripartitions;
        }
    }

    at(index){
        if(index < this.cells.length) return this.cells[index];
        else return null;
    }

    size(){
        return this.cells.length;
    }
}

class cols_div extends array_of_div {
    constructor(containing_div, cells_ripartitions, prctg_or_pixel = 1){   
        super(containing_div, cells_ripartitions, prctg_or_pixel);
        this.init(prctg_or_pixel);
    }

    static get_equispaced(containing_div, numb_of_elements){
        return new cols_div(containing_div, get_ones(numb_of_elements));
    };

    init(prctg_or_pixel){
        this.wrapping_div.style.display = "flex";
        for(let i=0; i<this.Sizes.length; i++){
            let created_div = document.createElement("div");
            if(prctg_or_pixel == 1){
                created_div.style.width = this.Sizes[i] + "%";
            }
            else{
                created_div.style.width = this.Sizes[i] + "px";
            }
            created_div.style.height = "100%";
            this.wrapping_div.appendChild(created_div);
            this.cells.push(created_div);
        }
    };
}

class rows_div extends array_of_div {
    constructor(containing_div, cells_ripartitions, prctg_or_pixel = 1){   
        super(containing_div, cells_ripartitions, prctg_or_pixel);
        this.init(prctg_or_pixel);
    }
    
    static get_equispaced(containing_div, numb_of_elements){
        return new rows_div(containing_div, get_ones(numb_of_elements));
    };

    init(prctg_or_pixel){
        for(let i=0; i<this.Sizes.length; i++){
            let created_div = document.createElement("div");
            created_div.style.width = "100%";
            if(prctg_or_pixel == 1){
                created_div.style.height = this.Sizes[i] + "%";
            }
            else{
                created_div.style.height = this.Sizes[i] + "px";
            }
            this.wrapping_div.appendChild(created_div);
            this.cells.push(created_div);
        }
    }
}

class matrix_div {
    constructor(containing_div, row_ripartition, col_ripartition, prctg_or_pixel = 1){
        let rows_temp = new rows_div(containing_div, row_ripartition, prctg_or_pixel);
        this.__rows=[];
        for(let i=0; i<rows_temp.size(); i++){
            this.__rows.push(new cols_div(rows_temp.at(i), col_ripartition, prctg_or_pixel));
        }

    }

    at(row, col){
        if(row < this.__rows.length) return this.__rows[row].at(col);
        else return null;
    }

    row_numb(){
        return this.__rows.length;
    }

    col_numb(){
        return this.__rows[0].size();
    }

    static get_equispaced_matrix(containing_div, n_row, n_col){
        return new matrix_div(containing_div , get_ones(n_row) , get_ones(n_col));
    }
}

function put_in_the_middle(wrapping_div, to_insert_div, margin, center_flag = 'C'){
    let marg = [margin, 1-margin , margin];
    if(center_flag == 'C'){
        let mat_temp = new matrix_div(wrapping_div , marg , marg);
        mat_temp.at(1,1).appendChild(to_insert_div);
    }
    else if(center_flag == 'W'){
        let cols_temp = new cols_div(wrapping_div, marg);
        cols_temp.at(1).appendChild(to_insert_div);
    }
    else if(center_flag == 'H'){
        let rows_temp = new rows_div(wrapping_div, marg);
        rows_temp.at(1).appendChild(to_insert_div);
    }
}

function put_in_the_middle2(wrapping_div, to_insert_div, margins){
    let marg1 = [margins[1], 1-margins[1] , margins[1]];
    let marg2 = [margins[0], 1-margins[0] , margins[0]];
    let mat_temp = new matrix_div(wrapping_div , marg1 , marg2);
    mat_temp.at(1,1).appendChild(to_insert_div);
}
