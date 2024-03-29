//
// Created by antony on 9/29/19.
//

#ifndef CPPLAB3_ELEMENT_H
#define CPPLAB3_ELEMENT_H

#include <iostream>
#include <csignal>

namespace cpplab3v13{

    int dialog();
    void signal_handler(int);

    enum conditions{LOW, HIGH, X};
    enum types{IN, OUT, IM};

    struct connection{
        types type;
        int sockets[3];
        conditions condition;
        connection(){
            type = IM;
            condition = X;
            for(int & socket : sockets){
                socket = -1;
            }
        }
        connection& set_cond(int);
    };

    class element{
    private:
        int conns;
        connection *cs;
    public:
        explicit element(int in = 1, int out = 1);  // constructors
        explicit element(connection);     // used in overloaded +=
        element(connection* arr, int sum);
        element(const element&); // copying
        element(element&&) noexcept;  // moving

        element& disconnect_conn(int which);  // modificators
        element& add_conn(connection newcomer);
        element& delete_conn(int which);

        element& operator ()(int which, int whereto);  // connect conns
        connection& operator [](int);    // set state
        connection operator [](int) const;   // get state
        element& operator +=(const element&);  // for adding a connection
        element& operator =(const element&); // overloaded =
        element& operator =(element&&) noexcept;    // moving =

        friend std::istream& operator >> (std::istream&, element&);  // total reorg of stats
        friend std::ostream& operator << (std::ostream&, const element&);  // print all

        ~element(){ delete [] cs; }  // destructor
    };

    template <class T>
    int input_number(T &a, std::istream& stream){
        auto previous_handler = std::signal(SIGINT, signal_handler);
        if (previous_handler == SIG_ERR) {
            std::cerr << "signal set failed!\n";
            return EXIT_FAILURE;
        }
        stream >> a;
        if(stream.eof()) return 0;    // eof
        if (!stream.good()){
            stream.clear();
            stream.ignore(100, '\n');
            return -1; // input error
        }
        return 1;
    }

    int d_add_conn(element&),
        d_del_conn(element&),
        d_show_all(element&),
        d_disconnect_conn(element&),
        d_connect_conn(element&),
        d_change_all_states(element&),
        d_print_conn_state(element&),
        d_set_conn_state(element&);
}

#endif //CPPLAB3_ELEMENT_H
