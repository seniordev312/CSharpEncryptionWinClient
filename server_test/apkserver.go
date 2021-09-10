package main

import (
	"bytes"
	"context"
	"crypto/aes"
	"crypto/cipher"
	"crypto/rand"
	"crypto/rsa"
	"crypto/sha1"
	"crypto/x509"
	"database/sql"
	"encoding/base64"
	"encoding/hex"
	"encoding/json"
	"encoding/pem"
	"errors"
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"path"
	"time"

	_ "github.com/go-sql-driver/mysql"
)

const (
	username = "root"
	password = "135207Alex"
	hostname = "127.0.0.1:3306"
	dbname   = "webApp"
)

var gSourceFilePath string
var db *sql.DB

type ApkResponse struct {
	Key string
	Id  string
}

type ApkData struct {
	UserEmail  		string
	Password  		string
	Id  			string
	File_key		string
	File_passcode 	string
	File_challenge 	string
}

type UserInfo struct {
	UserName  				string
	UserEmail  				string
	Password				string

	FirstName				string
	LastName				string
	HomePhone				string
	BusinessPhone			string
	StickerNumber			string
	FilterLevel				string
	IsOptionalRestrictions 	bool
	IsCamera				bool
	IsGallery				bool
	IsMusic					bool
	IsSdCard				bool
	IsFileManager			bool
	IsBtFileTransfer		bool
	IsOutgoingCallsWL		bool
	IsIncomingCallsWL		bool
	IsParentalBlock			bool
	ParentalBlockCode		string
	ParentalPhoneNumber		string
}

type deviceInfo struct {
	UserEmail  		string
	Password  		string
	Imei  			uint64
	Manufacturer	string
	Model			string
	Version			int
	SerialNumber	string
	PhoneNUmber		string
}

func dsn(dbName string) string {
	return fmt.Sprintf("%s:%s@tcp(%s)/%s", username, password, hostname, dbName)
}

func dbConnection() (error) {
	db_, err := sql.Open("mysql", dsn(""))
	db = db_
	if err != nil {
		log.Printf("Error %s when opening DB\n", err)
		return err
	}
	//defer db.Close()

	ctx, cancelfunc := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancelfunc()
	res, err := db.ExecContext(ctx, "CREATE DATABASE IF NOT EXISTS "+dbname)
	if err != nil {
		log.Printf("Error %s when creating DB\n", err)
		return err
	}
	no, err := res.RowsAffected()
	if err != nil {
		log.Printf("Error %s when fetching rows", err)
		return err
	}
	log.Printf("rows affected %d\n", no)

	db.Close()
	db, err = sql.Open("mysql", dsn(dbname))
	if err != nil {
		log.Printf("Error %s when opening DB", err)
		return err
	}
	//defer db.Close()

	db.SetMaxOpenConns(20)
	db.SetMaxIdleConns(20)
	db.SetConnMaxLifetime(time.Minute * 5)

	ctx, cancelfunc = context.WithTimeout(context.Background(), 5*time.Second)
	defer cancelfunc()
	err = db.PingContext(ctx)
	if err != nil {
		log.Printf("Errors %s pinging DB", err)
		return err
	}
	log.Printf("Connected to DB %s successfully\n", dbname)
	return nil
}

func createTable(query string) error {
	ctx, cancelfunc := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancelfunc()
	res, err := db.ExecContext(ctx, query)
	if err != nil {
		log.Printf("Error %s when creating product table", err)
		return err
	}
	rows, err := res.RowsAffected()
	if err != nil {
		log.Printf("Error %s when getting rows affected", err)
		return err
	}
	log.Printf("Rows affected when creating table: %d", rows)
	return nil
}

func checkCredentionals(email string, password string) (bool, string, error) {
	var name string
	var storedPassw string
	var result bool

	query := "SELECT user_name, password FROM userData WHERE user_email=?"
	ctx, cancelfunc := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancelfunc()
	err := db.QueryRowContext(ctx, query, email).Scan(&name, &storedPassw)

	switch {
	case err == sql.ErrNoRows:
		log.Printf("wrong UserInfo: email - %s\n", email)
	case err != nil:
		log.Fatalf("query error: %v\n", err)
		return false, name, err
	default:
	}

	if err != sql.ErrNoRows && storedPassw == password {
		result = true
	} else {
		result = false
	}
	return result, name, err
}

func updateCredentionals(cred UserInfo) error {
	query := "UPDATE userData set first_name = ?, last_name = ?, home_phone = ?, business_phone = ?, " +
			"sticker_number = ?, filter_level = ?, is_optional_restrictions = ?, is_camera = ?, " +
			"is_gallery = ?, is_music = ?, is_sd_card = ?, is_file_manager = ?, is_bt_file_transfer = ?, " +
			"is_outgoing_calls_wl = ?, is_incoming_calls_wl = ?, is_parental_block = ?, " +
			"parental_block_code = ?, parental_phone_number = ? where user_email = ?"
	ctx, cancelfunc := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancelfunc()
	stmt, err := db.PrepareContext(ctx, query)
	if err != nil {
		log.Printf("Error %s when preparing SQL statement", err)
		return err
	}
	defer stmt.Close()
	_, err = stmt.ExecContext(ctx, cred.FirstName, cred.LastName, cred.HomePhone, cred.BusinessPhone,
								cred.StickerNumber, cred.FilterLevel, cred.IsOptionalRestrictions,
								cred.IsCamera, cred.IsGallery, cred.IsMusic, cred.IsSdCard, cred.IsFileManager,
								cred.IsBtFileTransfer, cred.IsOutgoingCallsWL, cred.IsIncomingCallsWL,
								cred.IsParentalBlock, cred.ParentalBlockCode, cred.ParentalPhoneNumber, cred.UserEmail)
	if err != nil {
		log.Printf("Error %s when inserting row into userData table", err)
		return err
	}

	return nil
}

func insertCredentionals(cred UserInfo) error {
	query := "INSERT INTO userData(user_email, user_name, password) VALUES (?, ?, ?)"
	ctx, cancelfunc := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancelfunc()
	stmt, err := db.PrepareContext(ctx, query)
	if err != nil {
		log.Printf("Error %s when preparing SQL statement", err)
		return err
	}
	defer stmt.Close()
	_, err = stmt.ExecContext(ctx, cred.UserEmail, cred.UserName, cred.Password)
	if err != nil {
		log.Printf("Error %s when inserting row into userData table", err)
		return err
	}

	return nil
}

func insertDevice(device deviceInfo) error {
	query := "INSERT INTO devicesData(user_email, imei, manufacturer, model, version, serialNumber, phoneNumber) VALUES (?, ?, ?, ?, ?, ?, ?)"
	ctx, cancelfunc := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancelfunc()
	stmt, err := db.PrepareContext(ctx, query)
	if err != nil {
		log.Printf("Error %s when preparing SQL statement", err)
		return err
	}
	defer stmt.Close()
	_, err = stmt.ExecContext(ctx, device.UserEmail, device.Imei, device.Manufacturer,
								device.Model, device.Version, device.SerialNumber, device.PhoneNUmber)
	if err != nil {
		log.Printf("Error %s when inserting row into devicesData table", err)
		return err
	}

	return nil
}

func insertRedeemInivation(inivationIn string) error {
	query := "INSERT INTO redeemInvitations(invitation) VALUES (?)"
	ctx, cancelfunc := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancelfunc()
	stmt, err := db.PrepareContext(ctx, query)
	if err != nil {
		log.Printf("Error %s when preparing SQL statement", err)
		return err
	}
	defer stmt.Close()
	_, err = stmt.ExecContext(ctx, inivationIn)
	if err != nil {
		log.Printf("Error %s when inserting row into redeemInvitations table", err)
		return err
	}

	return nil
}

func insertApkData(apkDataVar ApkData) error {
	query := "INSERT INTO apkData(apk_id, file_passcode, file_challenge, file_key) VALUES (?, ?, ?, ?)"
	ctx, cancelfunc := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancelfunc()
	stmt, err := db.PrepareContext(ctx, query)
	if err != nil {
		log.Printf("Error %s when preparing SQL statement", err)
		return err
	}
	defer stmt.Close()
	_, err= stmt.ExecContext(ctx, apkDataVar.Id, apkDataVar.File_passcode, apkDataVar.File_challenge, apkDataVar.File_key)
	if err != nil {
		log.Printf("Error %s when inserting row into apkKeys table", err)
		return err
	}
	return nil
}

func insertApkKeys(apkDataVar ApkResponse) error {
	query := "INSERT INTO apkKeys(apk_id, apk_key) VALUES (?, ?)"
	ctx, cancelfunc := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancelfunc()
	stmt, err := db.PrepareContext(ctx, query)
	if err != nil {
		log.Printf("Error %s when preparing SQL statement", err)
		return err
	}
	defer stmt.Close()
	_, err= stmt.ExecContext(ctx, apkDataVar.Id, apkDataVar.Key)
	if err != nil {
		log.Printf("Error %s when inserting row into apkKeys table", err)
		return err
	}
	return nil
}

func BytesToPublicKey(pubPem []byte) *rsa.PublicKey {
	block, _ := pem.Decode(pubPem)
	if block == nil {
		log.Println("Failed pem Decode")
		return nil
	}

	pub, err := x509.ParsePKCS1PublicKey(block.Bytes)
	if err != nil {
		log.Println("Failed ParsePKCS1PublicKey: " + err.Error())
		return nil
	}

	return pub
}

func generateUniqueFileName(ext string) string {

	uniq := generateAES256Key()
	sh := sha1.New()
	sh.Write(uniq)
	bs := sh.Sum(nil)
	h := hex.EncodeToString(bs)
	return h + ext
}

func apkDataHandler(w http.ResponseWriter, r *http.Request) {
	log.Println("method:", r.Method)

	//read
	switch r.Method {
	case "GET":
		fmt.Fprint(w, "use POST for request")
	case "POST":
		var apkData_ ApkData
		var unmarshalErr *json.UnmarshalTypeError

		decoder := json.NewDecoder(r.Body)
		err := decoder.Decode(&apkData_)
		if err != nil {
			if errors.As(err, &unmarshalErr) {
				log.Println( "Bad Request. Wrong Type provided for field "+unmarshalErr.Field)
			} else {
				log.Println( "Bad Request "+err.Error())
			}
			return
		}

		result, _, _ := checkCredentionals(apkData_.UserEmail, apkData_.Password)
		apkData_.Id = generateUniqueFileName("")
		if result {
			err = insertApkData(apkData_)
		}

		w.Header().Set("Content-type", "application/json")
		w.WriteHeader(http.StatusCreated)
		if result && err == nil {
			w.Write([]byte(apkData_.Id))
		} else {
			w.Write([]byte("0"))
		}
	default:
		fmt.Fprintf(w, "Use GET or POST")
	}
}

func userInfoHandler(w http.ResponseWriter, r *http.Request) {
	log.Println("method:", r.Method)

	//read
	switch r.Method {
	case "GET":
		fmt.Fprint(w, "use POST for request")
	case "POST":
		var userInfo_ UserInfo
		var unmarshalErr *json.UnmarshalTypeError

		decoder := json.NewDecoder(r.Body)
		err := decoder.Decode(&userInfo_)
		if err != nil {
			if errors.As(err, &unmarshalErr) {
				log.Println( "Bad Request. Wrong Type provided for field "+unmarshalErr.Field)
			} else {
				log.Println( "Bad Request "+err.Error())
			}
			return
		}

		result, _, _ := checkCredentionals(userInfo_.UserEmail, userInfo_.Password)

		if result {
			err = updateCredentionals(userInfo_)
		}

		w.Header().Set("Content-type", "application/json")
		w.WriteHeader(http.StatusCreated)
		if result && err == nil {
			w.Write([]byte("1"))
		} else {
			w.Write([]byte("0"))
		}
	default:
		fmt.Fprintf(w, "Use GET or POST")
	}
}

func deviceHandler(w http.ResponseWriter, r *http.Request) {
	log.Println("method:", r.Method)

	//read
	switch r.Method {
	case "GET":
		fmt.Fprint(w, "use POST for request")
	case "POST":
		var device deviceInfo
		var unmarshalErr *json.UnmarshalTypeError

		decoder := json.NewDecoder(r.Body)
		err := decoder.Decode(&device)
		if err != nil {
			if errors.As(err, &unmarshalErr) {
				log.Println( "Bad Request. Wrong Type provided for field "+unmarshalErr.Field)
			} else {
				log.Println( "Bad Request "+err.Error())
			}
			return
		}

		result, _, _ := checkCredentionals(device.UserEmail, device.Password)

		if result {
			err = insertDevice(device)
		}

		w.Header().Set("Content-type", "application/json")
		w.WriteHeader(http.StatusCreated)
		if result && err == nil {
			w.Write([]byte("1"))
		} else {
			w.Write([]byte("0"))
		}
	default:
		fmt.Fprintf(w, "Use GET or POST")
	}
}

func loginHandler(w http.ResponseWriter, r *http.Request) {
	log.Println("method:", r.Method)

	//read
	switch r.Method {
	case "GET":
		fmt.Fprint(w, "use POST for request")
	case "POST":
		type MessageIn struct {
			UserEmail string
			Password string
		}
		var m MessageIn
		var unmarshalErr *json.UnmarshalTypeError

		decoder := json.NewDecoder(r.Body)
		err := decoder.Decode(&m)
		if err != nil {
			if errors.As(err, &unmarshalErr) {
				log.Println( "Bad Request. Wrong Type provided for field "+unmarshalErr.Field)
			} else {
				log.Println( "Bad Request "+err.Error())
			}
			return
		}

		result, name, err := checkCredentionals(m.UserEmail, m.Password)

		w.Header().Set("Content-type", "application/json")
		w.WriteHeader(http.StatusCreated)
		//Encode to base64 encoded string
		type MessageOut struct {
			Res, Name string
		}
		var loginResponse MessageOut
		loginResponse.Name = name
		if result {
			loginResponse.Res = "1"
		} else {
			loginResponse.Res = "0"
		}
		json.NewEncoder(w).Encode(&loginResponse)
	default:
		fmt.Fprintf(w, "Use GET or POST")
	}
}

func signUpHandler(w http.ResponseWriter, r *http.Request) {
	log.Println("method:", r.Method)

	//read
	switch r.Method {
	case "GET":
		fmt.Fprint(w, "use POST for request")
	case "POST":
		var cred UserInfo
		var unmarshalErr *json.UnmarshalTypeError

		body, err := ioutil.ReadAll(r.Body)
		err = json.Unmarshal(body, &cred)
		if err != nil {
			if errors.As(err, &unmarshalErr) {
				log.Println( "Bad Request. Wrong Type provided for field "+unmarshalErr.Field)
			} else {
				log.Println( "Bad Request "+err.Error())
			}
		}
		w.Header().Set("Content-type", "application/json")
		w.WriteHeader(http.StatusCreated)
		if err != nil {
			w.Write([]byte("0"))
		} else {
			err = insertCredentionals (cred)
			if err != nil {
				w.Write([]byte("0"))
			} else {
				w.Write([]byte("1"))
			}
		}
	default:
		fmt.Fprintf(w, "Use GET or POST")
	}
}

func invitationHandler(w http.ResponseWriter, r *http.Request) {
	log.Println("method:", r.Method)

	//read
	switch r.Method {
	case "GET":
		fmt.Fprint(w, "use POST for request")
	case "POST":
		invitation, err := ioutil.ReadAll(r.Body)
		if err != nil {
			fmt.Fprintf(w, "Failed read body")
			return
		}

		query := "SELECT invitation FROM redeemInvitations WHERE invitation=?"
		ctx, cancelfunc := context.WithTimeout(context.Background(), 5*time.Second)
		defer cancelfunc()
		err = db.QueryRowContext(ctx, query, invitation).Scan(&invitation)

		switch {
		case err == sql.ErrNoRows:
			log.Printf("wrong redeemInvitation - %s\n", invitation)
		case err != nil:
			log.Fatalf("query error: %v\n", err)
			return
		default:
			log.Printf("correct redeemInvitation - %s\n", invitation)
		}

		w.Header().Set("Content-type", "application/json")
		w.WriteHeader(http.StatusCreated)
		if err == sql.ErrNoRows {
			w.Write([]byte("0"))
		} else {
			w.Write([]byte("1"))
		}

	default:
		fmt.Fprintf(w, "Use GET or POST")
	}
}

func encryptHandler(w http.ResponseWriter, r *http.Request) {
	log.Println("method:", r.Method)

	//read
	switch r.Method {
	case "GET":
		fmt.Fprint(w, "use POST for request")
	case "POST":
		//read pub key from header filed

		publicKey, err := ioutil.ReadAll(r.Body)
		if err != nil {
			fmt.Fprintf(w, "Failed read body")
			return
		}

		log.Printf("public-key:%s", string(publicKey))

		rsaPublicKey := BytesToPublicKey(publicKey)
		if rsaPublicKey == nil {
			fmt.Fprintf(w, "Failed BytesToPublicKey")
			return
		}

		sourceFilePath := gSourceFilePath

		//Generate aes256 key
		aesKey := generateAES256Key()
		encodedFilePath := generateUniqueFileName("_encoded_" + path.Ext(sourceFilePath))

		ret := encryptFileCBC(sourceFilePath, encodedFilePath, aesKey)
		if ret != true {
			return
		}

		sh := sha1.New()
		aesKeyEncoded, _ := rsa.EncryptOAEP(sh, rand.Reader, rsaPublicKey, aesKey, nil)

		//Encode to base64 encoded string
		apkResponse := &ApkResponse{Key: base64.StdEncoding.EncodeToString(aesKeyEncoded), Id: encodedFilePath}

		w.Header().Set("Content-type", "application/json")
		w.WriteHeader(http.StatusCreated)
		json.NewEncoder(w).Encode(apkResponse)

		insertApkKeys(*apkResponse)
	default:
		fmt.Fprintf(w, "Use GET or POST")
	}
}

func generateAES256Key() []byte {
	//32 for aes256
	key := make([]byte, 32)
	_, err := rand.Read(key)
	if err != nil {
		log.Println(err)
		return nil
	}
	return key
}

func PKCS7Padding(ciphertext []byte, blockSize int) []byte {
	padding := blockSize - len(ciphertext)%blockSize
	padtext := bytes.Repeat([]byte{byte(padding)}, padding)
	return append(ciphertext, padtext...)
}

func PKCS7UnPadding(origData []byte) []byte {
	length := len(origData)
	unpadding := int(origData[length-1])
	return origData[:(length - unpadding)]
}

func decryptCBC(key []byte, encryptData []byte) []byte {
	block, err := aes.NewCipher(key)
	if err != nil {
		log.Println(err)
		return nil
	}

	blockSize := aes.BlockSize

	iv := encryptData[:blockSize]
	encryptData = encryptData[blockSize:]

	mode := cipher.NewCBCDecrypter(block, iv)
	mode.CryptBlocks(encryptData, encryptData)
	encryptData = PKCS7UnPadding(encryptData)

	return encryptData
}

func decryptFileCBC(encodedFile string, decodedFile string, key []byte, iv []byte) {
	data, err := ioutil.ReadFile(encodedFile)
	if err != nil {
		return
	}

	decoded := decryptCBC(key, data)
	if decoded == nil {
		return
	}

	err = ioutil.WriteFile(decodedFile, decoded, 777)
	if err != nil {
		log.Println(err)
		return
	}
}

func encryptCBC(key []byte, rawData []byte) []byte {
	b, err := aes.NewCipher(key)
	if err != nil {
		return nil
	}

	blockSize := aes.BlockSize
	rawData = PKCS7Padding(rawData, blockSize)

	ciphertext := make([]byte, blockSize+len(rawData))
	iv := ciphertext[:blockSize]
	if _, err := io.ReadFull(rand.Reader, iv); err != nil {
		return nil
	}

	mode := cipher.NewCBCEncrypter(b, iv)
	mode.CryptBlocks(ciphertext[blockSize:], rawData)
	return ciphertext
}

func encryptFileCBC(sourceFile string, targetFile string, key []byte) bool {
	//Read data from file
	data, err := ioutil.ReadFile(sourceFile)
	if err != nil {
		log.Println(err)
		return false
	}
	log.Println("[OK] ReadFile")

	enc := encryptCBC(key, data)

	if enc == nil {
		log.Println("[FAILED] encryptCBC")
		return false
	}
	log.Println("[OK] encryptCBC")

	err = ioutil.WriteFile(targetFile, enc, 777)
	if err != nil {
		log.Println(err)
		return false
	}

	log.Println("[OK] WriteFile")

	return true
}

func downloadHandler(w http.ResponseWriter, r *http.Request) {
	log.Println("method:", r.Method)
	keys, ok := r.URL.Query()["id"]

	if !ok || len(keys[0]) < 1 {
		log.Println("Url Param 'id' is missing")
		return
	}
	id := keys[0]

	log.Printf("Request file id:'%s'", id)
	if id != "" {
		http.ServeFile(w, r, id)
	}

	os.Remove(id)
}

func main() {

	sourcePtr := flag.String("source", "", "file path to source file for encryption")
	flag.Parse()
	gSourceFilePath = *sourcePtr

	if gSourceFilePath == "" {
		log.Println("Setup source file. --source=<file_path> ")
		return
	}

	err := dbConnection()
	if err != nil {
		log.Printf("Error %s when getting db connection", err)
		return
	}

	log.Printf("Successfully connected to database")

	err = createTable(`CREATE TABLE IF NOT EXISTS userData(user_email varchar(500), user_name text, password text, 
							first_name text, last_name text, home_phone text, business_phone text,
							sticker_number text, filter_level text, is_optional_restrictions boolean, 
							is_camera boolean, is_gallery boolean, is_music boolean, is_sd_card boolean, 
							is_file_manager boolean, is_bt_file_transfer boolean, is_outgoing_calls_wl boolean,
							is_incoming_calls_wl boolean, is_parental_block boolean, 
							parental_block_code text, parental_phone_number text, UNIQUE (user_email))`)
	if err != nil {
		log.Printf("Create userData table failed with error %s", err)
		return
	}

	err = createTable(`CREATE TABLE IF NOT EXISTS devicesData(user_email varchar(500), imei BIGINT, manufacturer text, 
							model text, version int, serialNumber text, phoneNumber text)`)
	if err != nil {
		log.Printf("Create devicesData table failed with error %s", err)
		return
	}

	err = createTable(`CREATE TABLE IF NOT EXISTS apkKeys(apk_id varchar(500) UNIQUE, apk_key text)`)
	if err != nil {
		log.Printf("Create apkKeys table failed with error %s", err)
		return
	}

	err = createTable(`CREATE TABLE IF NOT EXISTS apkData(apk_id varchar(500) UNIQUE, file_passcode BLOB, file_challenge BLOB, file_key BLOB)`)
	if err != nil {
		log.Printf("Create apkData table failed with error %s", err)
		return
	}

	err = createTable(`CREATE TABLE IF NOT EXISTS redeemInvitations(invitation varchar(500) UNIQUE)`)
	if err != nil {
		log.Printf("Create redeemInvitation table failed with error %s", err)
		return
	}

	var count int

	err = db.QueryRow("SELECT COUNT(*) FROM redeemInvitations").Scan(&count)
	switch {
	case err != nil:
		log.Fatal(err)
	default:
		if count == 0 {
			a := []string{"1234", "2341", "1432"}
			for _, s := range a {
				err = insertRedeemInivation(s)
				if err != nil {
					log.Printf("Insert product failed with error %s", err)
					return
				}
			}
		}
	}

	http.HandleFunc("/encrypt", encryptHandler)
	http.HandleFunc("/download", downloadHandler)
	http.HandleFunc("/invitation", invitationHandler)
	http.HandleFunc("/signUp", signUpHandler)
	http.HandleFunc("/login", loginHandler)
	http.HandleFunc("/device", deviceHandler)
	http.HandleFunc("/userInfo", userInfoHandler)
	http.HandleFunc("/apkData", apkDataHandler)

	log.Fatal(http.ListenAndServe(":8080", nil))

}
