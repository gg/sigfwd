// Copyright 2010 Edd Dawson.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <QtCore/QApplication>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>

#include <boost/signals2.hpp>

#include <cstdlib>
#include <cassert>
#include <functional>

#include <sigfwd/support/boost/bind.hpp>
#include <sigfwd/support/boost/signals2.hpp>
#include <sigfwd/connect.hpp>


struct percentage
{
    int value;

    percentage() : value(0) {}
    void set(int newvalue) { value = newvalue; }
};

void display(const percentage &pc)
{
    QMessageBox::information(0, "Report", "Percentage: " + QString::number(pc.value) + " %");
}

void die(double)
{
    abort();
}

// It seems that the signal2::signal must be wrapped with boost::function
// as demonstrated below.
template <typename SignalType>
void ConnectToSignal(QObject* qobject, const char* qt_signal_signature,
                     const SignalType& signal) {
  typedef typename SignalType::signature_type SignalSignature;
  sigfwd::connection con = sigfwd::connect(
      qobject, qt_signal_signature, boost::function<SignalSignature>(ref(signal)));
  assert(con.status == sigfwd::connected);
}

int main(int argc, char **argv)
{
    // Create a window containing a couple of widgets.
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("sigfwd demo");

    QSlider *slider = new QSlider(Qt::Horizontal, &window);
    slider->setMinimum(0);
    slider->setMaximum(100);
    slider->setTickPosition(QSlider::TicksBelow);

    QPushButton *button = new QPushButton("Report", &window);

    QHBoxLayout *layout = new QHBoxLayout(&window);
    layout->addWidget(new QLabel(QString::number(slider->minimum()), &window));
    layout->addWidget(slider);
    layout->addWidget(new QLabel(QString::number(slider->maximum()), &window));
    layout->addSpacing(20);
    layout->addWidget(button);


    // Now use sigfwd to create some connections.
    using boost::bind; using boost::ref;

    percentage pc;
    boost::signals2::signal<void ()> dummy_signal;
    boost::signals2::signal<void ()> button_clicked;

    {
        boost::signals2::connection conn = dummy_signal.connect(ref(button_clicked));
        assert(conn.connected());
    }

    {
        const sigfwd::connection con = sigfwd::connect(
            slider, SIGNAL(valueChanged(int)), bind(&percentage::set, ref(pc), _1));
        assert(con.status == sigfwd::connected);
    }

    {
#if 0
        ConnectToSignal(button, SIGNAL(clicked()), button_clicked);
#endif
        sigfwd::connect(button, SIGNAL(clicked()), ref(button_clicked));
    }

    {
        boost::signals2::connection conn = button_clicked.connect(bind(display, ref(pc)));
        assert(conn.connected());
    }

    {
        const sigfwd::connection con = sigfwd::connect(slider, SIGNAL(valueChanged(int)), die);
        // incompatible signatures, connection won't be made
        assert(con.status == sigfwd::sigs_incompatible);
    }

    // And finally show the window...
    window.show();
    return app.exec();
}
