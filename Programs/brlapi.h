/*
 * BRLTTY - A background process providing access to the Linux console (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2003 by The BRLTTY Team. All rights reserved.
 *
 * BRLTTY comes with ABSOLUTELY NO WARRANTY.
 *
 * This is free software, placed under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation.  Please see the file COPYING for details.
 *
 * Web Page: http://mielke.cc/brltty/
 *
 * This software is maintained by Dave Mielke <dave@mielke.cc>.
 */

/** \file
 * \brief types, defines and functions prototypes for \e BrlAPI
 */

#ifndef _BRLAPI_H
#define _BRLAPI_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* this is for UINT32_MAX */
#include <inttypes.h>
/* The type size_t is defined there! */
#include <unistd.h>

/** \defgroup brlapi_connection Connecting to BrlAPI
 *
 * Before calling any other function of the library, calling
 * brlapi_initializeConnection() is needed to establish a connection to
 * \e BrlAPI 's server.
 * When the connection is no more needed, brlapi_closeConnection() must be
 * called to close the connection.
 *
 * @{ */

/** Default port number on which connections to \e BrlAPI can be established */
#define BRLAPI_SOCKETPORT "35751"

/** \e brltty 's settings directory
 *
 * This is where authentication key and driver-dependant key names are found 
 * for instance. */
#define BRLAPI_ETCDIR "/etc/brltty"

/** Default name of the file containing \e BrlAPI 's authentication key
 *
 * This name is relative to BRLAPI_ETCDIR */
#define BRLAPI_AUTHFILE "brlapi-key"

/** Final path for default authentication key file */
#define BRLAPI_AUTHNAME BRLAPI_ETCDIR "/" BRLAPI_AUTHFILE

/** \brief Settings structure for \e BrlAPI connection
 *
 * This structure holds every parameter needed to connect to \e BrlAPI: in which
 * file the authentication key can be found and to which computer to connect.
 *
 * \par Examples:
 * \code
 * brlapi_settings_t settings;
 *
 * settings.authKey="/etc/brltty/brlapi-key";
 * settings.hostName="foo";
 * \endcode
 *
 * \e libbrlapi will read authentication key from file \p /etc/brltty/brlapi-key
 * and connect to the machine called "foo", on the default TCP port.
 *
 * \code
 * settings.hostName="10.1.0.2";
 * \endcode
 *
 * lets directly enter an IP address instead of a machine name.
 *
 * \code
 * settings.hostName=":4321";
 * \endcode
 *
 * lets \e libbrlapi connect to the local computer, on TCP port 4321
 *
 * \sa brlapi_initializeConnection() brlapi_loadAuthKey()
 * */
typedef struct
{
  /** To get authenticated, \e libbrlapi has to tell the \e BrlAPI server a
   * secret key, for security reasons. This is the path to the file which 
   * holds it; it will hence have to be readable by the application.
   *
   * Setting \c NULL defaults it to local installation setup. */
  char *authKey;

  /** this tells where the \e BrlAPI server resides: it might be listening on
   * another computer, on any TCP port. It should look like "foo:1234", which
   * means TCP port number 1234 on computer called "foo".
   * \note Please check that resolving this name works before complaining
   *
   * Settings \c NULL defaults it to localhost, using the local installation's
   * default TCP port. */
  char *hostName;
} brlapi_settings_t;

/* brlapi_initializeConnection */
/** open a socket and connect it to \e BrlAPI 's server
 *
 * This function first loads an authentication key as specified in settings.
 * It then creates a TCP socket and connects it to the specified machine, on
 * the specified port. It writes the authentication key on the socket and
 * waits for acknowledgement.
 *
 * \return the file descriptor, or -1 on error
 *
 * \note The file descriptor is returned in case the client wants to
 * communicate with the server without using \e libbrlapi functions. If it uses
 * them however, it won't have to pass the file descriptor later, since the
 * library keeps a copy of it
 *
 * \par Example:
 * \code
 * if (brlapi_initializeConnection(&settings)<0) {
 *  fprintf(stderr,"couldn't connect to BrlAPI!\n");
 *  exit(1);
 * }
 * \endcode
 *
 * \par Errors:
 * \e BrlAPI might not be on this TCP port, the host name might not be
 * resolvable, the authentication may fail,...
 * 
 * \param clientSettings this gives the connection parameters, as described
 * in brlapi_settings_t. If \c NULL, defaults values are used, so that it is
 * generally a good idea to give \c NULL as default, and only fill a
 * brlapi_settings_t structure when the user gave parameters to the program
 * for instance.
 * \param usedSettings if not \c NULL, parameters which were actually used are
 * stored here, if the application ever needs them.
 *
 * \sa
 * brlapi_settings_t
 * brlapi_loadAuthKey()
 * brlapi_writePacket()
 * brlapi_readPacket()
 */
int brlapi_initializeConnection(const brlapi_settings_t *clientSettings, brlapi_settings_t *usedSettings);

/* brlapi_closeConnection */
/** \ingroup brlapi_connection
 * Cleanly close the socket 
 *
 * This function locks until a closing acknowledgement is received from the
 * server. The socket is then freed, so the file descriptor
 * brlapi_initializeConnection() gave has no meaning any more
 */
void brlapi_closeConnection(void);

/* brlapi_loadAuthKey */
/** Load an authentication key from the given file
 *
 * Calling this function shouldn't be needed if brlapi_initializeConnection
 * is used.
 *
 * \param filename gives the full path of the file ;
 * \param authlength gives the size of the \e auth buffer ;
 * \param auth is a buffer where the function will store the authentication key
 *
 * \return the size of the key, -1 on error
 *
 * \sa brlapi_settings_t, brlapi_initializeConnection */
int brlapi_loadAuthKey(const char *filename, int *authlength, void *auth);

/* brlapi_getControllingTty */
/** Return the number of the caller's controlling terminal
 *
 * \return -1 if unknown or on error */
int brlapi_getControllingTty(void);

/** @} */

/** \defgroup brlapi_info Getting Terminal information
 * \brief How to get information about the connected Terminal
 *
 * Before using Raw mode or key codes, the application should always check the
 * type of the connected terminal, to be sure it is really the one it expects.
 *
 * One should also check for display size, so as to adjust further displaying
 * on it.
 * @{
 */

/* brlapi_getDriverId */
/** Identify the driver used by \e brltty
 *
 * This function will return the \e brltty 2-chars code of the driver
 * currently in use.
 *
 * \note The returned string is static, so that further calls will modify it */
char *brlapi_getDriverId(void);

/* brlapi_getDriverName */
/** return the complete name of the driver used by \e brltty
 *
 * This function will return the whole name of the braille terminal, if
 * available.
 *
 * \note The returned string is static, so that further calls will modify it */
char *brlapi_getDriverName(void);

/* brlapi_getDisplaySize */
/** return the size of the braille display */
int brlapi_getDisplaySize(unsigned int *x, unsigned int *y);

/** @} */

/** \brief Key binding structure
 * \ingroup brlapi_keys
 *
 * This holds settings for the key binding mechanism. Calling
 * brlapi_getTty() with it will load bindings from file 
 * \c "$HOME/BRLAPI_HOMEKEYDIR/client-xy\#\#BRLAPI_HOMEKEYEXT"
 * where \c xy is the driver code, as returned by brlapi_getDriverId()
 */
typedef struct
{
  /** application name
   *
   * this is used to build the name of the file which will be read for bindings.
   */
  const char *client;
} brlapi_keybinding_t;

/** \defgroup brlapi_tty Tty getting & leaving
 * \brief How to take control of ttys for direct braille display / read
 *
 * Before being able to write on the braille display, the application must tell
 * the server which tty it will handle. Some checking is done just to be sure
 * that only one client gets control of each tty.
 *
 * The application must also specify how braille keys will be delivered to it.
 * Two ways are possible: KEYCODES and COMMANDS 
 *
 * - KEYCODES are specific to each braille driver, since the raw keycode, as
 *   defined in the driver will be given for each key press.
 *   Using them leads to building highly driver-dependent applications, which
 *   can yet sometimes be useful to mimic existing proprietary applications
 *   for instance.
 * - COMMANDS means that applications will get exactly the same values as
 *   \e brltty. This allows driver-independent clients, which will hopefuly
 *   be nice to use with a lot of different terminals
 * \sa brlapi_readKey() brlapi_readCommand() brlapi_readBinding()
 * @{
 */

/* brlapi_getTty */
/** ask for some tty, with some key mechanism
 *
 * \param tty
 * - If tty>0, application takes control of the specified tty;
 * - if tty==0, the library tries to determine which tty the application is
 * running on and take control of this one.
 *
 * \param how tells whether the application wants brlapi_readKey() to return
 * keycodes or \e brltty commands: either BRLKEYCODES or BRLCOMMANDS;
 *
 * \param keybinding tells settings for bindings. This is only necessary if
 * brlapi_readBinding() will be called; else \c NULL can be given.
 *
 * \return 0 on success, -1 on error
 */
int brlapi_getTty(uint32_t tty, uint32_t how, brlapi_keybinding_t *keybinding);

/** ask for raw driver keycodes */
#define BRLKEYCODES ((uint32_t) 1)
/** ask for \e brltty commands */
#define BRLCOMMANDS ((uint32_t) 2)

/* brlapi_leaveTty */
/** stop controlling the tty
 *
 * \return 0 on success, -1 on error.
 */
int brlapi_leaveTty(void);

/** @} */

/** \defgroup brlapi_write Writing on the braille display
 * \brief write text to the braille display 
 * 
 * Once brlapi_getTty() was called, the application can call brlapi_writeBrl()
 * to write things on the braille display.
 * @{ */

/* brlapi_writeBrl */
/** Write the given \\0-terminated string to the braille display
 *
 * If the string is too long, it is cut. If it's too short, spaces are appended.
 *
 * \param cursor gives the cursor position; if less than or
 * equal to 0 or greater than the display width, no cursor is shown at all;
 * \param str points on the string to be displayed.
 *
 * \return 0 on success, -1 on error.
 */
int brlapi_writeBrl(uint32_t cursor, const char *str);

/* brlapi_writeBrlDots */
/** Write the given brlx*brly sized array to the display
 *
 * \param dots points on the memory address which contains
 * the dot information.
 *
 * \return 0 on success, -1 on error.
 */
int brlapi_writeBrlDots(const char *dots);

/** @} */

/** \defgroup brlapi_keys Reading key presses
 * \brief How to read key presses from the braille terminal
 *
 * Once brlapi_getTty() is called, the application can call brlapi_readKey(),
 * brlapi_readCommand() or brlapi_readBinding() to read keypresses. Not
 * everyone can be called, it depends on parameters given to brlapi_getTty().
 *
 * key presses are buffered, so that calling brlapi_readKey() in non-blocking
 * mode from times to times should suffice.
 *
 * @{
 */

/** buffer size
 *
 * key presses won't be lost provided no more than BRL_KEYBUF_SIZE key presses
 * are done between two calls to brlapi_read* if a call to another function is
 * done in the meanwhile (which needs somewhere to put them before being able
 * to get responses from the server)
 */
#define BRL_KEYBUF_SIZE 256

/** type for key codes
 *
 * Its size is 32 bits, so driver implementors have to restrict themselves to
 * a 32 bit space. */
typedef uint32_t brl_keycode_t;

/** brl_keycode_t's bigest value
 *
 * As defined in \c <inttypes.h> */
#define BRL_KEYCODE_MAX ((brl_keycode_t) (UINT32_MAX))

/* brlapi_readKey */
/** Read a key from the braille keyboard
 *
 * This function returns raw keycodes, as specified by the terminal driver. It
 * generally corresponds to the code the terminal tells to the driver.
 *
 * This should only be used to application which are dedicated to a particular
 * braille terminal. Hence, checking the terminal type thanks to a call to
 * brlapi_getDriverId() or even brlapi_getDriverName() before getting tty
 * control is a pretty good idea.
 *
 * It can be called only if BRLKEYCODES was given to brlapi_getTty()
 *
 * \param block tells whether the call should block until a key is pressed (1)
 *  or should only probe key presses (0),
 * \param code holds the key code if a key press is indeed read.
 *
 * \return -1 on error and *code is then undefined, 0 if block was 0 and no
 *  key was pressed so far, or 1 and *code holds the key code.
 */
int brlapi_readKey(int block, brl_keycode_t *code);

/* brlapi_readCommand */
/** Read a command from the braille keyboard
 *
 * This function returns \e brltty commands, as described in
 * \c <brltty/brldefs.h> .
 * These are hence pretty driver-independant, and should be used by default
 * when no other option is possible.
 *
 * It can be called only if BRLCOMMANDS was given to brlapi_getTty()
 *
 * \param block tells whether the call should block until a key is pressed (1)
 *  or should only probe key presses (0).
 * \param code holds the command code if a key press is indeed read.
 *
 * \return -1 on error and *code is then undefined, 0 if block was 0 and no
 *  key was pressed so far, or 1 and *code holds the command code.
 */
int brlapi_readCommand(int block, brl_keycode_t *code);

/** \e BrlAPI 's $HOME settings directory
 *
 * this directory will contain per-user configuration
 */
#define BRLAPI_HOMEKEYDIR ".brlkeys"

/** \e BrlAPI 's key binding suffix
 */
#define BRLAPI_HOMEKEYEXT ".kbd"

/** Prefix for driver-dependant keynames header files
 *
 * Driver-dependant keynames header files are stored in
 * BRLAPI_ETCDIR/BRLAPI_ETCKEYFILE-xy.h
 * where \c xy is the driver code, as returned by brlapi_getDriverId() */
#define BRLAPI_ETCKEYFILE "brlkeys"

/* brlapi_readBinding */
/** read a key binding from the braille keyboard
 *
 * This function returns a command name, as bound in user's config file:
 * if the read is successful, a pointer on a string is
 * returned, this is the string defined in $HOME/.brlkeys/appli-xy.kbd for the
 * key which was read, else \c NULL is returned 
 *
 * \param block tells whether the call should block until a key is pressed (1)
 *  or should only probe key presses (0).
 * \param code holds the key string if a key press is indeed read.
 *
 * It can be called only if a brlapi_keybinding_t structure was filled and
 * given to brlapi_getTty()
 *
 * \return -1 on error and *code is then undefined, 0 if block was 0 and no
 *  key was pressed so far, or 1 and *code holds the key string.
 */
int brlapi_readBinding(int block, const char **code);

/* brlapi_ignoreKeys */
/** Ignore some keypresses from the braille keyboard
 *
 * This function asks the server to give keys between x and y to \e brltty,
 * rather than returning them to the application via
 * brlapi_read(Key|Command|Binding)()
 *
 * \note The given codes are either raw keycodes if BRLKEYCODES was given to
 * brlapi_getTty(), or \e brltty commands if BRLCOMMANDS was given. */
int brlapi_ignoreKeys(brl_keycode_t x, brl_keycode_t y);

/* brlapi_unignoreKeys */
/** Unignore some keypresses from the braille keyboard
 *
 * This function asks the server to return keys between x and y to the
 * application, and not give them to \e brltty.
 *
 * \note You shouldn't ask the server to give you keypresses which are usually
 * used to switch between TTYs, unless you really know what you are doing !
 *
 * \note The given codes are either raw keycodes if BRLKEYCODES was given to
 * brlapi_getTty(), or \e brltty commands if BRLCOMMANDS was given. */
int brlapi_unignoreKeys(brl_keycode_t x, brl_keycode_t y);

/** @} */

/** \defgroup brlapi_raw Raw Mode
 * \brief Raw Mode mechanism
 *
 * If the application wants to directly talk to the braille terminal, it should
 * use Raw Mode. In this special mode, the driver gives the whole control of the
 * terminal to it: \e brltty doesn't work any more.
 *
 * For this, it simply has to call brlapi_getRaw(), then brlapi_sendRaw()
 * and brlapi_recvRaw(), and finaly give control back thanks to
 * brlapi_leaveRaw()
 *
 * Special care of the terminal should be taken, since one might completely
 * trash the terminal's data, or even lock it ! The application should always
 * check for terminal's type thanks to brlapi_getDriverId().
 *
 * @{
 */

/* brlapi_getRaw */
/** Switch to Raw mode
 * \return 0 on success, -1 on error */
int brlapi_getRaw(void);

/* brlapi_leaveRaw */
/** Leave Raw mode
 * \return 0 on success, -1 on error */
int brlapi_leaveRaw(void);

/* brlapi_sendRaw */
/** Send a Raw Packet
 * 
 * \param buf points on the packet;
 * \param size holds the packet size.
 * \return 0 on success, -1 on error */
int brlapi_sendRaw(const unsigned char *buf, size_t size);

/* brlapi_recvRaw */
/** Get a Raw packet
 *
 * \param buf points on a buffer where the function will store the received
 * packet;
 * \param size holds the buffer size.
 * \return its size, -1 on error, or -2 on EOF */
int brlapi_recvRaw(unsigned char *buf, size_t size);

/** @} */

/** \defgroup brlapi_error Error handling
 * \brief how to handle errors which might very well happen
 *
 * This is still under development. A brlapi_errno variable should be available,
 * as well as a brlapi_errlist[] array and a brlapi_perror() function.
 * @{ */

/* Error codes */ 
#define BRLERR_NOMEM                    1  /**< Not enough memory */
#define BRLERR_TTYBUSY                  2  /**< Already a connection running in this tty */
#define BRLERR_UNKNOWN_INSTRUCTION      3  /**< Not implemented in protocol */
#define BRLERR_ILLEGAL_INSTRUCTION      4  /**< Forbiden in current mode */
#define BRLERR_INVALID_PARAMETER        5  /**< Out of range or have no sense */
#define BRLERR_INVALID_PACKET           6  /**< Invalid size */
#define BRLERR_RAWNOTSUPP               7  /**< Raw mode not supported by loaded driver */
#define BRLERR_KEYSNOTSUPP              8  /**< Reading of key codes not supported by loaded driver */
#define BRLERR_CONNREFUSED              9  /**< Connection refused */
#define BRLERR_OPNOTSUPP                10  /**< Operation not supported */
/** @} */

/** \defgroup brlapi_protocol BrlAPI's protocol
 * \brief Instructions and constants for \e BrlAPI 's protocol
 * 
 * These are defines for the protocol between \e BrlAPI 's server and clients.
 * Understanding is not needed to use the \e BrlAPI library, so reading this
 * is not needed unless really wanting to connect to \e BrlAPI without
 * \e BrlAPI 's library.
 *
 * @{ */

/** Maximum packet size for packets exchanged on sockets and with braille
 * terminal */
#define BRLAPI_MAXPACKETSIZE 512

/** type for packet type. Only unsigned can cross networks, 32bits */
typedef uint32_t brl_type_t;

#define BRLPACKET_AUTHKEY           'K'    /**< Authentication key          */
#define BRLPACKET_BYE               'B'    /**< Bye                         */
#define BRLPACKET_GETDRIVERID       'd'    /**< Ask which driver is used    */
#define BRLPACKET_GETDRIVERNAME     'n'    /**< Ask which driver is used    */
#define BRLPACKET_GETDISPLAYSIZE    's'    /**< Dimensions of brl display   */
#define BRLPACKET_GETTTY            't'    /**< asks for a specified tty    */
#define BRLPACKET_LEAVETTY          'L'    /**< release the tty             */
#define BRLPACKET_KEY               'k'    /**< braille key                 */
#define BRLPACKET_COMMAND           'c'    /**< braille command             */
#define BRLPACKET_MASKKEYS          'm'    /**< Mask a key-range            */
#define BRLPACKET_UNMASKKEYS        'u'    /**< Unmask key range            */
#define BRLPACKET_WRITE             'W'    /**< Write On Braille Display    */
#define BRLPACKET_WRITEDOTS         'D'    /**< Write Dots On Braille Display  */
#define BRLPACKET_STATWRITE         'S'    /**< Write Status Cells          */
#define BRLPACKET_GETRAW            '*'    /**< Enter in raw mode           */
#define BRLPACKET_LEAVERAW          '#'    /**< Leave raw mode              */
#define BRLPACKET_PACKET            'p'    /**< Raw packets                 */
#define BRLPACKET_ACK               'A'    /**< Acknowledgement             */
#define BRLPACKET_ERROR             'E'    /**< Error in protocol           */

/** Magic number to give when sending a BRLPACKET_GETRAW packet */
#define BRLRAW_MAGIC (0xdeadbeefL) 

/* brlapi_writePacket */
/** Send a packet to \e BrlAPI server
 *
 * This function is for internal use, but one might use it if one really knows
 * what one is doing...
 * 
 * \e type should only be one of the above defined BRLPACKET_*.
 *
 * The syntax is the same as write()'s.
 * 
 * \return 0 on success, -1 on failure */
int brlapi_writePacket(int fd, brl_type_t type, const void *buf, size_t size);

/* brlapi_readPacket */
/** Read a packet from \e BrlAPI server
 *
 * This function is for internal use, but one might use it if one really knows
 * what one is doing...
 *
 * \e type is where the function will store the packet type; it should always
 * be one of the above defined BRLPACKET_* (or else something very nasty must
 * have happened :/).
 *
 * The syntax is the same as read()'s.
 *
 * \return packet's size, -2 if \c EOF occured, -1 on error */
int brlapi_readPacket(int fd, brl_type_t *type, void *buf, size_t size);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _BRLAPI_H */
