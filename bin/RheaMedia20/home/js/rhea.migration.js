class RheaMigration {
    // XML DOM object
    #tempFolder;
    #jsonConfig;
    #importedPath;
    #db;

    constructor(fld, cfg, path, db) {
        this.#tempFolder = fld;
        this.#jsonConfig = cfg;
        this.#importedPath = path;
        this.#db = db;
    }

    convert = async () => {
        pleaseWait_addMessage("importing general settings...");
        await this.#setPageMenu();

        pleaseWait_addMessage("removing previous configured selections...");
        await this.#deletePageMMI();

        pleaseWait_addMessage("updating supported languages...");
        await this.#updateGeneralSettings();

        pleaseWait_addMessage("importing selections data...");
        await this.#setPageMMI();

        pleaseWait_addMessage("importing customized images...");
        await this.#diffImages();

        pleaseWait_addMessage("importing various messages...");
        await this.#setVarious();

        pleaseWait_addMessage("importing splash screen configurations...");
        await this.#setSplashScreen();
    };

    // P R I V A T E   M E T H O D S

    /** Sets the "Page Manu" properties*/
    #setPageMenu = async () => {
        const q = // sql 
            RheaMigration.#trim(`
                UPDATE pagemenu SET 
                    icon_numRow = ${this.#jsonConfig.Cino_generalSettings.NumTableSelRows}, 
                    icon_numIconPerRow = ${this.#jsonConfig.Cino_generalSettings.NumTableSelColumns}, 
                    gotoPageStandbyAfterMSec = ${this.#jsonConfig.Cino_generalSettings.Cino_MenuPageTimeout}000 
                WHERE HIS_ID=1 
            `);

        await this.#db.q(q);
    };

    /** Removes all selections info */
    #deletePageMMI = async () => {
        await this.#db.q('DELETE FROM pagemenu_mmi;');
        await this.#db.q('DELETE FROM lang WHERE UID="MMI_NAME" OR UID="MMI_DESCR" OR UID="MMI_NUTRI";');
    };

    /** Updates supported languages */
    #updateGeneralSettings = async () => {
        await this.#db.q("UPDATE generalset SET  allowedLang='GB', defaultLang='GB';");
    };

    /** Sets the "Page Manu MMI" properties  */
    #setPageMMI = async () => {
        const len = this.#jsonConfig.Cino_selection.SN.length;
        const selections = this.#jsonConfig.Cino_selection;

        await this.#db.exec('ALTER TABLE pagemenu_mmi ADD ndName TEXT(120);');

        for (let i = 0; i < len; i++) {
            let linkedSelection = '';

            for (let j = 0; j < 11; j++) {
                linkedSelection += `${i + 1}, `
            }

            linkedSelection += `${i + 1}`

            const q = // sql 
                RheaMigration.#trim(`
                    INSERT INTO pagemenu_mmi (
                        HIS_ID, 
                        PROGR, 
                        selNum, 
                        pageMenuImg, 
                        pageConfirmImg, 
                        optionAEnabled, 
                        optionBEnabled, 
                        allowedCupSize, 
                        linkedSelection, 
                        defaultSelectionOption, 
                        name, 
                        ndName, 
                        bCustomBtnCupAppeance
                    ) VALUES (
                        1, 
                        ${i + 1},
                        ${i + 1},
                        '${selections.SI[i]}',
                        '${selections.SI[i]}',
                        0,
                        0,
                        100,
                        '${linkedSelection.replaceAll(" ", "")}',
                        0,
                        '${selections.SN[i]}',
                        '',
                        0
                    )
                `);

            // Insert row into DB
            await this.#db.exec(q);

            // Retrieved UID of prev query
            const dbData = await this.#db.q(`SELECT UID FROM pagemenu_mmi WHERE selNum = ${i + 1} `);
            const data = this.#getDBData(dbData);

            const UID = data.find(d => d)?.UID;

            // Insert row into DB
            if (UID) {
                await this.#db.exec(`INSERT INTO lang (UID, ISO, What, Message) VALUES ( 'MMI_NAME', 'GB', ${UID}, '${selections.SN[i]}' );`);
                await this.#db.exec(`INSERT INTO lang (UID, ISO, What, Message) VALUES ( 'MMI_DESCR', 'GB', ${UID}, '${selections.SD[i]}' );`);
            }
        }
    };

    /** Copies image files if different between old GUI */
    #diffImages = async () => {
        const IMG_PATH = `${this.#tempFolder}/img-drinks`

        // Retrieves web folder
        const leaf = this.#tempFolder.split('\\').pop().split('/').pop();
        const path = this.#tempFolder.replace(`/${leaf}`, '');

        const res = await this.#getFileList(IMG_PATH);

        if (res.success) {
            const DEFAULT_IMGS = res.result;
            const OLD_IMGS = this.#jsonConfig.Cino_selection.SI;

            // Intersection
            const existingImgs = OLD_IMGS.filter(x => DEFAULT_IMGS.includes(x));

            // Difference
            const diffImgs = OLD_IMGS.filter(x => !DEFAULT_IMGS.includes(x));

            const uploadsFolder = `${path}/upload`;

            // Copies existing files into uploads folder
            for (let i = 0; i < existingImgs.length; i++) {
                const file = existingImgs[i];

                await this.#fileCopy(IMG_PATH, uploadsFolder, file)
            }

            // Copies existing files into uploads folder
            for (let i = 0; i < diffImgs.length; i++) {
                const file = diffImgs[i];

                await this.#fileCopy(`${this.#importedPath}/images_drinks`, uploadsFolder, file)
            }
        }
    };

    /** Sets the "lang" properties */
    #setVarious = async () => {
        await this.#db.exec(`REPLACE INTO lang (UID, ISO, What, Message) VALUES ( 'LAB_CURRENCY_SIMBOL', 'GB', 'MSG', '${this.#jsonConfig.Cino_generalSettings.Cino_Text_Currency}' );`);
        await this.#db.exec(`REPLACE INTO lang (UID, ISO, What, Message) VALUES ( 'LAB_YOUR_DRINK_IS_BEING_PREPARED', 'GB', 'MSG', '${this.#jsonConfig.Cino_generalSettings.Cino_Text_SelPreparing}' );`);
        await this.#db.exec(`REPLACE INTO lang (UID, ISO, What, Message) VALUES ( 'LAB_YOUR_DRINK_IS_READY', 'GB', 'MSG', '${this.#jsonConfig.Cino_generalSettings.Cino_Text_SelEnd}' );`);
    };

    /** Sets the "lang" properties */
    #setSplashScreen = async () => {
        const BK_IMAGES = this.#jsonConfig.Cino_standby.BK;

        if (BK_IMAGES && BK_IMAGES.length) {
            // Retrieves web folder
            const leaf = this.#tempFolder.split('\\').pop().split('/').pop();
            const path = this.#tempFolder.replace(`/${leaf}`, '');

            await this.#db.q(`DELETE FROM pageStandby_images WHERE HIS_ID=1;`);

            for (let i = 0; i < BK_IMAGES.length; i++) {
                const img = BK_IMAGES[i];

                await this.#fileCopy(`${this.#importedPath}/images_backgrounds`, `${path}/upload`, img)
                await this.#db.exec(`REPLACE INTO pageStandby_images (HIS_ID, PROGR, filename) VALUES ( 1, ${i + 1}, '../upload/${img}' );`);
            }
        }
    };

    #getFileList = (path) => {
        return new Promise((resolve, reject) => {
            rhea.ajax("FSList", { "path": path, "jolly": "*.png *.gif" })
                .then(function (result) {
                    resolve({
                        success: true,
                        result: JSON.parse(result).fileList.split('§')
                    });
                })
                .catch(function (result) {
                    reject({ success: false });
                });
        });
    };

    /**
     * Copies a file to the uploads folder
     * @param {string} sourcePath Absolute path with file name
     */
    #fileCopy(sourcePath, destinationPath, fileName) {
        return new Promise((resolve, reject) => {
            rhea.ajax('FSCopy', { "pSRC": sourcePath, "fSRC": fileName, "pDST": destinationPath, "fDST": fileName })
                .then((result) => {
                    resolve({
                        success: true,
                        data: result
                    });
                })
                .catch(err => {
                    reject({
                        success: false,
                        data: err
                    });
                });
        });
    };

    /**
     * Retrieves data from DB object as Array<object> mapped as key value
     * @param {DBObject} dbData The db data object
     * @returns {Array<object>} key valued array list
     */
    #getDBData(dbData) {
        if (!dbData || dbData.err || !dbData.data || !dbData.data.length) { return; }

        const data = new Array();

        for (let row = 0; row < dbData.getNumRows(); row++) {
            const rowData = {};

            for (let col = 0; col < dbData.getNumCols(); col++) {
                rowData[dbData.getColName(col)] = dbData.val(row, col);
            }

            data.push(rowData);
        }

        return data;
    };

    /**
     * Removes special chars from db query string
     * @param {String} str String to be cleaned
     * @returns {String} A trimmed string
     */
    static #trim = (str) => {
        return str.replace(/\n/g, ' ')
            .replace(/  +/g, ' ')
            .trim();
    };
}
