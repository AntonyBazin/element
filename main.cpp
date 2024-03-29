#include "element.h"

using namespace cpplab3v13;

int main() {
    int rc;
    //auto *ptr = new connection[7];
    int (*function_ptr[])(element&) = {nullptr, d_add_conn, d_del_conn,
                                       d_show_all, d_disconnect_conn,
                                       d_connect_conn,d_change_all_states,
                                       d_print_conn_state, d_set_conn_state};


    /*for(int i = 0; i < 5; ++i){
        ptr[i].type = i < 2 ? IN : OUT;
    }*/
    element elem;
    while((rc = dialog())){
        if(!function_ptr[rc](elem)) break;
    }
    //delete [] ptr;
    return 0;
}