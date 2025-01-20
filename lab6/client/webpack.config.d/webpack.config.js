module.exports = {
	devServer: {
        proxy: {
            '/napi/*': {
                target: 'ws://localhost:3001',
                ws: true,
            },
        },
    }
};