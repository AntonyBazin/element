//
// Created by antony on 9/29/19.
//

#include "element.h"

namespace cpplab3v13{

    const char *messages[] = {"0. Quit                ",
                              "1. Add connection",
                              "2. Delete connection",
                              "3. Print info",
                              "4. Disconnect something",
                              "5. Connect something",
                              "6. Total state change",
                              "7. Get connection state",
                              "8. Set connection state"};

    const int NMsgs = sizeof(messages)/sizeof(messages[0]);

    element::element(int in, int out) {
        conns = 0;
        cs = nullptr;
        for(int i = 0; i < (in + out); ++i){
            connection con;
            con.type = i < in ? IN : OUT;
            add_conn(con);
        }
    }

    element::element(connection *arr, int sum) {
        conns = 0;
        cs = nullptr;
        for(int i = 0; i < sum; ++i){
            if(arr[i].type == IM) arr[i].type = IN;
            for(int j = 0; j < 3; ++j){
                arr[i].sockets[j] = -1;
            }
            add_conn(arr[i]);
        }
    }

    element::element(const connection cn) {
        conns = 0;
        cs = nullptr;
        add_conn(cn);
    }

    element::element(const element & elem) : conns(elem.conns){
        cs = new connection[conns];
        for(int i = 0; i < conns; ++i){
            cs[i] = elem.cs[i];
        }
    }

    element::element(element && elem) noexcept : conns(elem.conns), cs(elem.cs){
        elem.cs = nullptr;
    }


    element &element::add_conn(const connection newcomer) {
        ++conns;
        if(cs == nullptr){
            cs = new connection[1];
            cs[0] = newcomer;
            return *this;
        }
        connection *old = cs;
        cs = new connection[conns];
        for(int i = 0; i < (conns - 1); ++i){
            cs[i] = old[i];
        }
        delete [] old;
        cs[conns - 1] = newcomer;
        return *this;
    }

    element &element::disconnect_conn(const int which) {
        if(which < 0 || which >= conns)
            throw std::runtime_error("invalid connection index");
        if(cs[which].type == IM)
            throw std::runtime_error("there is no such connection");

        if(cs[which].type == IN){
            if(cs[which].sockets[0] != -1){
                for(int i = 0; i < 3; ++i){
                    if(cs[cs[which].sockets[0]].sockets[i] == which)
                        cs[cs[which].sockets[0]].sockets[i] = -1;
                }
                cs[which].sockets[0] = -1;
                bool lonely = true;
                for(int j = 0; j < 3; ++j){
                    if(cs[cs[which].sockets[0]].sockets[j] != -1){
                        lonely = false;
                        break;
                    }
                }
                if(lonely) cs[cs[which].sockets[0]].condition = X;
            }
        } else{
            for(int i = 0; i < 3; ++i){ //OUT-type has 3 available sockets
                if(cs[which].sockets[i] != -1){   // find a connected socket
                    for(int j = 0; j < conns; ++j){ //find where to
                        cs[cs[which].sockets[i]].sockets[0] = -1; //OUT can only be connected to IN
                        cs[cs[which].sockets[i]].condition = X;//and IN has only one available socket
                    }
                }
            }
            for(int i = 0; i < 3; ++i){
                cs[which].sockets[i] = -1;
            }
        }
        cs[which].condition = X;
        return *this;
    }

    element &element::delete_conn(const int which) {
        if(which < 0 || which >= conns)
            throw std::runtime_error("invalid connection index");
        if(cs[which].type == IM)
            throw std::runtime_error("there is no such connection");
        for(int i = 0; i < conns; ++i){
            for(int j = 0; j < 3; ++j){
                if(cs[i].sockets[j] == which)
                    cs[i].sockets[j] = -1;
            }
        }
        cs[which] = cs[--conns];
        cs[conns].type = IM;
        cs[conns].condition = X;
        for(int i = 0; i < 3; ++i){
            if(cs[which].sockets[i] != -1){
                for(int j = 0; j < 3; ++j){
                    if(cs[cs[which].sockets[i]].sockets[j] == conns){
                        cs[cs[which].sockets[i]].sockets[j] = which;
                    }
                }

            }
        }
        return *this;
    }

    connection& element::operator[](const int index) {
        if(index < 0 || index >= conns)
            throw std::runtime_error("invalid connection index");
        if(cs[index].type == IM)
            throw std::runtime_error("there is no such connection");
        return cs[index];
    }

    connection element::operator[](const int index) const { //const connection
        if(index < 0 || index >= conns)
            throw std::runtime_error("invalid connection index");
        if(cs[index].type == IM)
            throw std::runtime_error("there is no such connection");
        return cs[index];
    }

    element &element::operator()(int which, int whereto) {
        if(which < 0 || which >= conns)
            throw std::runtime_error("invalid connection index");
        if(cs[which].type == IM)
            throw std::runtime_error("there is no such connection");
        if(whereto < 0 || whereto >= conns)
            throw std::runtime_error("invalid connection index");
        if(cs[whereto].type == IM)
            throw std::runtime_error("there is no such connection");
        if(cs[whereto].type == cs[which].type)
            throw std::runtime_error("you can\'t connect same types");

        if(cs[which].type == OUT){
            int h = which;
            which = whereto;
            whereto = h;
        }

        for(int i = 0; i < 3; ++i){
            if(cs[whereto].sockets[i] == -1){
                break;
            }
            if(i == 2)
                throw std::runtime_error("no place to plug in in target");
        }

        if(cs[which].sockets[0] != -1)
            throw std::runtime_error("this connection is busy! disconnect it first!");
        else
            cs[which].sockets[0] = whereto;

        for(int i = 0; i < 3; ++i){
            if(cs[whereto].sockets[i] == -1){
                cs[whereto].sockets[i] = which;
                break;
            }
        }

        return *this;
    }

    element &element::operator+=(const element& elem) {
        int tmp = conns;
        conns += elem.conns;
        connection *old = cs;
        cs = new connection[conns];
        for(int i = 0; i < conns; ++i){
            cs[i] = i < tmp ? old[i] : elem.cs[i - tmp];
        }
        delete [] old;
        return *this;
    }

    std::ostream &operator <<(std::ostream& s, const element& elem) {
        s << "info about all existing connections:" << std::endl;

        for(int i = 0; i < elem.conns; ++i){
            s << "connection #" << (i+1) << ":" << std::endl << "Condition: ";
            switch(elem.cs[i].condition){
                case 0:
                    s << "LOW; ";
                    break;
                case 1:
                    s << "HIGH; ";
                    break;
                default:
                    s << "NOT DEFINED; ";
                    break;
            }
            s << "type: ";
            switch(elem.cs[i].type){
                case 0:
                    s << "INPUT; ";
                    break;
                case 1:
                    s << "OUTPUT; ";
                    break;
                default:
                    s << "IMAGINARY; ";
                    break;
            }
            s << "connected to elements:";
            for(int j = 0; j < 3; ++j){
                if(elem.cs[i].sockets[j] != -1){
                    s << " " << (elem.cs[i].sockets[j] + 1);
                }
            }
            s << std::endl << std::endl;
        }
        return s;
    }

    std::istream &operator >>(std::istream& s, element& elem) {
        int c, rc;
        for(int i = 0; i < elem.conns; ++i){
            bool lonely = true;
            for(int j = 0; j < 3; ++j){
                if(elem.cs[i].sockets[j] != -1){
                    lonely = false;
                    break;
                }
            }
            if(lonely) continue;

            do{
                rc = input_number(c, s);
                if(rc == 1) break;
                if(!rc) return s;  // eof
                std::cout << "incorrect input, please, try again:";
            }while(rc < 0);

            switch(c){
                case 0:
                    elem.cs[i].set_cond(0);
                    break;
                case 1:
                    elem.cs[i].set_cond(1);
                    break;
                default:
                    elem.cs[i].set_cond(2);
                    break;
            }
        }
        return s;
    }

    element &element::operator =(const element& elem) {
        if(this != &elem){
            conns = elem.conns;
            delete [] cs;
            cs = new connection[conns];
            for(int i = 0; i < conns; ++i){
                cs[i] = elem.cs[i];
            }
        }
        return *this;
    }

    element &element::operator =(element&& elem) noexcept {
        conns = elem.conns;
        connection *h = cs;
        cs = elem.cs;
        elem.cs = h;
        return *this;
    }


    void signal_handler(int signal){
        if (signal == SIGINT) {
            std::cerr << "SIGINT received\n";
        } else {
            std::cerr << "Unexpected signal: " << signal << "\n";
        }
        std::_Exit(EXIT_SUCCESS);
    }


    int dialog(){
        char *report = (char*)"";
        int rc, i, n;
        do{
            std::cout << report << std::endl;
            report = (char*)"You are wrong. Please, try again.";
            for(i = 0; i < NMsgs; ++i) {  //print list of alternatives
                std::cout << messages[i] << std::endl;
            }
            printf("Make your choice: ~ ");
            n = input_number(rc, std::cin);  //enter number of alternative
            if(!n) rc = 0;  //EOF == end of the program
        } while(rc < 0 || rc >= NMsgs);
        return rc;
    }

    int d_add_conn(element& elem){
        int rc, b;
        connection con;

        std::cout << "Input connection\'s type(1 for in, else for out):"
                  << std::endl;

        do{
            rc = input_number(b, std::cin);
            if(rc == 1) break;
            if(!rc) return 0;
            std::cout << "Incorrect input, please, try again:";
        }while(rc < 0);

        con.type = (b == 1 ? IN : OUT);
        try {
            elem += element(con);
        } catch (std::runtime_error &rt) {
            std::cout << rt.what() << std::endl;
        }
        return 1;
    }

    int d_del_conn(element& elem){
        int rc, b;

        std::cout << "Input connection\'s current id to delete:" << std::endl;

        do{
            rc = input_number(b, std::cin);
            if(rc == 1) break;
            if(!rc) return 0;
            std::cout << "Incorrect input, please, try again:";
        }while(rc < 0);

        try{
            elem.delete_conn(b - 1);
        } catch(std::runtime_error &rt){
            std::cout << rt.what() << std::endl;
        }
        return 1;
    }

    int d_show_all(element& elem){
        std::cout << elem;
        return 1;
    }

    int d_disconnect_conn(element& elem){
        int rc, a;

        std::cout << "Input connection\'s current id to disconnect:" << std::endl;
        do{
            rc = input_number(a, std::cin);
            if(rc == 1) break;
            if(!rc) return 0;
            std::cout << "Incorrect input, please, try again:";
        }while(rc < 0);

        try{
            elem.disconnect_conn(a - 1);
        } catch(std::runtime_error &rt){
            std::cout << rt.what() << std::endl;
        }
        return 1;
    }

    int d_connect_conn(element& elem){
        int rc, a, b;

        std::cout << "Input connection\'s current id to connect:" << std::endl;
        do{
            rc = input_number(a, std::cin);
            if(rc == 1) break;
            if(!rc) return 0;
            std::cout << "Incorrect input, please, try again:";
        }while(rc < 0);

        std::cout << "Input target connection id:" << std::endl;
        do{
            rc = input_number(b, std::cin);
            if(rc == 1) break;
            if(!rc) return 0;
            std::cout << "Incorrect input, please, try again:";
        }while(rc < 0);

        try{
            elem(a - 1, b - 1);
        } catch(std::runtime_error &rt){
            std::cout << rt.what() << std::endl;
        }
        return 1;
    }

    int d_change_all_states(element& elem){
        std::cin >> elem;
        return 1;
    }

    int d_print_conn_state(element& elem){
        int rc, b;
        const element elemc = elem;

        std::cout << "Input connection\'s current id to get its\' state:" << std::endl;
        do{
            rc = input_number(b, std::cin);
            if(rc == 1) break;
            if(!rc) return 0;
            std::cout << "Incorrect input, please, try again:";
        }while(rc < 0);

        try{
            std::cout << elemc[b - 1].condition;
        } catch(std::runtime_error &rt){
            std::cout << rt.what() << std::endl;
        }
        return 1;
    }

    int d_set_conn_state(element& elem){
        int rc, b, a;

        std::cout << "Input connection\'s current id to set state:" << std::endl;
        do{
            rc = input_number(b, std::cin);
            if(rc == 1) break;
            if(!rc) return 0;
            std::cout << "Incorrect input, please, try again:";
        }while(rc < 0);

        std::cout << "Input new state(0 - low, 1 - high, else - X)" << std::endl;
        do{
            rc = input_number(a, std::cin);
            if(rc == 1) break;
            if(!rc) return 0;
            std::cout << "Incorrect input, please, try again:";
        }while(rc < 0);

        try{
            elem[b - 1].set_cond(a);
        } catch(std::runtime_error &rt){
            std::cout << rt.what() << std::endl;
        }
        return 1;
    }

    connection &connection::set_cond(int new_state) {
        if(type == IM)
            throw std::runtime_error("there is no such connection");
        bool lonely = true;
        for(int i = 0; i < 3; ++i){
            if(sockets[i] != -1){
                lonely = false;
                break;
            }
        }
        if(lonely)
            throw std::runtime_error("only X state permitted for lonely connections");

        switch(new_state){
            case 0:
                condition = LOW;
                break;
            case 1:
                condition = HIGH;
                break;
            default:
                condition = X;
                break;
        }
        return *this;
    }
}
